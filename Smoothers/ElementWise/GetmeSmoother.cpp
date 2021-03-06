#include "GetmeSmoother.h"

#include <mutex>

#include "VertexAccum.h"
#include "Boundaries/Constraints/AbstractConstraint.h"
#include "DataStructures/MeshCrew.h"
#include "Evaluators/AbstractEvaluator.h"
#include "Measurers/AbstractMeasurer.h"

using namespace std;


// CUDA Drivers interface
void installCudaGetmeSmoother();


GetmeSmoother::GetmeSmoother() :
    AbstractElementWiseSmoother(
        {":/glsl/compute/Smoothing/ElementWise/GETMe.glsl"},
        installCudaGetmeSmoother),
    _lambda(0.78)
{

}

GetmeSmoother::~GetmeSmoother()
{

}

void GetmeSmoother::setElementProgramUniforms(
        const Mesh& mesh, cellar::GlProgram& program)
{
    AbstractElementWiseSmoother::setElementProgramUniforms(mesh, program);
    program.setFloat("Lambda", 0.78);
}

void GetmeSmoother::setVertexProgramUniforms(
        const Mesh& mesh, cellar::GlProgram& program)
{
    AbstractElementWiseSmoother::setElementProgramUniforms(mesh, program);
    program.setFloat("Lambda", 0.78);
}

void GetmeSmoother::printOptimisationParameters(
        const Mesh& mesh,
        OptimizationImpl& plotImpl) const
{
    AbstractElementWiseSmoother::printOptimisationParameters(mesh, plotImpl);
    plotImpl.addSmoothingProperty("Method Name", "GETMe");
    plotImpl.addSmoothingProperty("Lambda", to_string(_lambda));
}

void GetmeSmoother::smoothTets(
        Mesh& mesh,
        const MeshCrew& crew,
        size_t first,
        size_t last)
{
    const vector<MeshVert>& verts = mesh.verts;
    const vector<MeshTopo>& topos = mesh.topos;
    const vector<MeshTet>& tets = mesh.tets;

    for(int eId = first; eId < last; ++eId)
    {
        const MeshTet& tet = tets[eId];

        uint vi[] = {
            tet.v[0],
            tet.v[1],
            tet.v[2],
            tet.v[3],
        };

        glm::dvec3 vp[] = {
            verts[vi[0]].p,
            verts[vi[1]].p,
            verts[vi[2]].p,
            verts[vi[3]].p,
        };

        glm::dvec3 n[] = {
            glm::cross(vp[3]-vp[1], vp[2]-vp[1]),
            glm::cross(vp[3]-vp[2], vp[0]-vp[2]),
            glm::cross(vp[1]-vp[3], vp[0]-vp[3]),
            glm::cross(vp[1]-vp[0], vp[2]-vp[0]),
        };

        glm::dvec3 vpp[] = {
            vp[0] + n[0] * (_lambda / glm::sqrt(glm::length(n[0]))),
            vp[1] + n[1] * (_lambda / glm::sqrt(glm::length(n[1]))),
            vp[2] + n[2] * (_lambda / glm::sqrt(glm::length(n[2]))),
            vp[3] + n[3] * (_lambda / glm::sqrt(glm::length(n[3]))),
        };


        double volume = crew.measurer().tetVolume(crew.sampler(), vp, tet);
        double volumePrime = crew.measurer().tetVolume(crew.sampler(), vpp, tet);
        double absVolumeRation = glm::abs(volume / volumePrime);
        double volumeVar = glm::pow(absVolumeRation, 1.0/3.0);

        glm::dvec3 center = (1.0/4.0) * (
            vp[0] + vp[1] + vp[2] + vp[3]);

        vpp[0] = center + volumeVar * (vpp[0] - center);
        vpp[1] = center + volumeVar * (vpp[1] - center);
        vpp[2] = center + volumeVar * (vpp[2] - center);
        vpp[3] = center + volumeVar * (vpp[3] - center);

        if(topos[vi[0]].snapToBoundary->isConstrained())
            vpp[0] = (*topos[vi[0]].snapToBoundary)(vpp[0]);
        if(topos[vi[1]].snapToBoundary->isConstrained())
            vpp[1] = (*topos[vi[1]].snapToBoundary)(vpp[1]);
        if(topos[vi[2]].snapToBoundary->isConstrained())
            vpp[2] = (*topos[vi[2]].snapToBoundary)(vpp[2]);
        if(topos[vi[3]].snapToBoundary->isConstrained())
            vpp[3] = (*topos[vi[3]].snapToBoundary)(vpp[3]);

        double quality = crew.evaluator().tetQuality(crew.sampler(), crew.measurer(), vp, tet);
        double qualityPrime = crew.evaluator().tetQuality(crew.sampler(), crew.measurer(), vpp, tet);

        double weight = qualityPrime / (1.0 + quality);
        _vertexAccums[vi[0]]->addPosition(vpp[0], weight);
        _vertexAccums[vi[1]]->addPosition(vpp[1], weight);
        _vertexAccums[vi[2]]->addPosition(vpp[2], weight);
        _vertexAccums[vi[3]]->addPosition(vpp[3], weight);
    }
}

void GetmeSmoother::smoothPris(
        Mesh& mesh,
        const MeshCrew& crew,
        size_t first,
        size_t last)
{
    const vector<MeshVert>& verts = mesh.verts;
    const vector<MeshTopo>& topos = mesh.topos;
    const vector<MeshPri>& pris = mesh.pris;

    for(int eId = first; eId < last; ++eId)
    {
        const MeshPri& pri = pris[eId];

        uint vi[] = {
            pri.v[0],
            pri.v[1],
            pri.v[2],
            pri.v[3],
            pri.v[4],
            pri.v[5],
        };

        glm::dvec3 vp[] = {
            verts[vi[0]].p,
            verts[vi[1]].p,
            verts[vi[2]].p,
            verts[vi[3]].p,
            verts[vi[4]].p,
            verts[vi[5]].p,
        };

        glm::dvec3 aux[] = {
            (vp[0] + vp[1] + vp[2]) / 3.0,
            (vp[0] + vp[1] + vp[4] + vp[3]) / 4.0,
            (vp[1] + vp[2] + vp[5] + vp[4]) / 4.0,
            (vp[2] + vp[0] + vp[3] + vp[5]) / 4.0,
            (vp[3] + vp[4] + vp[5]) / 3.0,
        };

        glm::dvec3 n[] = {
            glm::cross(aux[1] - aux[0], aux[3] - aux[0]),
            glm::cross(aux[2] - aux[0], aux[1] - aux[0]),
            glm::cross(aux[3] - aux[0], aux[2] - aux[0]),
            glm::cross(aux[3] - aux[4], aux[1] - aux[4]),
            glm::cross(aux[1] - aux[4], aux[2] - aux[4]),
            glm::cross(aux[2] - aux[4], aux[3] - aux[4]),
        };

        double t = (4.0/5.0) * (1.0 - glm::pow(4.0/39.0, 0.25) * _lambda);
        double it = 1.0 - t;
        glm::dvec3 bases[] = {
            it * aux[0] + t * (aux[3] + aux[1]) / 2.0,
            it * aux[0] + t * (aux[1] + aux[2]) / 2.0,
            it * aux[0] + t * (aux[2] + aux[3]) / 2.0,
            it * aux[4] + t * (aux[3] + aux[1]) / 2.0,
            it * aux[4] + t * (aux[1] + aux[2]) / 2.0,
            it * aux[4] + t * (aux[2] + aux[3]) / 2.0,
        };


        // New positions
        glm::dvec3 vpp[] = {
            bases[0] + n[0] * (_lambda / glm::sqrt(glm::length(n[0]))),
            bases[1] + n[1] * (_lambda / glm::sqrt(glm::length(n[1]))),
            bases[2] + n[2] * (_lambda / glm::sqrt(glm::length(n[2]))),
            bases[3] + n[3] * (_lambda / glm::sqrt(glm::length(n[3]))),
            bases[4] + n[4] * (_lambda / glm::sqrt(glm::length(n[4]))),
            bases[5] + n[5] * (_lambda / glm::sqrt(glm::length(n[5]))),
        };


        double volume = crew.measurer().priVolume(crew.sampler(), vp, pri);
        double volumePrime = crew.measurer().priVolume(crew.sampler(), vpp, pri);
        double absVolumeRation = glm::abs(volume / volumePrime);
        double volumeVar = glm::pow(absVolumeRation, 1.0/3.0);

        glm::dvec3 center = (1.0/6.0) * (
            vp[0] + vp[1] + vp[2] + vp[3] + vp[4] + vp[5]);

        vpp[0] = center + volumeVar * (vpp[0] - center);
        vpp[1] = center + volumeVar * (vpp[1] - center);
        vpp[2] = center + volumeVar * (vpp[2] - center);
        vpp[3] = center + volumeVar * (vpp[3] - center);
        vpp[4] = center + volumeVar * (vpp[4] - center);
        vpp[5] = center + volumeVar * (vpp[5] - center);

        if(topos[vi[0]].snapToBoundary->isConstrained())
            vpp[0] = (*topos[vi[0]].snapToBoundary)(vpp[0]);
        if(topos[vi[1]].snapToBoundary->isConstrained())
            vpp[1] = (*topos[vi[1]].snapToBoundary)(vpp[1]);
        if(topos[vi[2]].snapToBoundary->isConstrained())
            vpp[2] = (*topos[vi[2]].snapToBoundary)(vpp[2]);
        if(topos[vi[3]].snapToBoundary->isConstrained())
            vpp[3] = (*topos[vi[3]].snapToBoundary)(vpp[3]);
        if(topos[vi[4]].snapToBoundary->isConstrained())
            vpp[4] = (*topos[vi[4]].snapToBoundary)(vpp[4]);
        if(topos[vi[5]].snapToBoundary->isConstrained())
            vpp[5] = (*topos[vi[5]].snapToBoundary)(vpp[5]);


        double quality = crew.evaluator().priQuality(crew.sampler(), crew.measurer(), vp, pri);
        double qualityPrime = crew.evaluator().priQuality(crew.sampler(), crew.measurer(), vpp, pri);

        double weight = qualityPrime / (1.0 + quality);
        _vertexAccums[vi[0]]->addPosition(vpp[0], weight);
        _vertexAccums[vi[1]]->addPosition(vpp[1], weight);
        _vertexAccums[vi[2]]->addPosition(vpp[2], weight);
        _vertexAccums[vi[3]]->addPosition(vpp[3], weight);
        _vertexAccums[vi[4]]->addPosition(vpp[4], weight);
        _vertexAccums[vi[5]]->addPosition(vpp[5], weight);
    }
}

void GetmeSmoother::smoothHexs(
        Mesh& mesh,
        const MeshCrew& crew,
        size_t first,
        size_t last)
{
    const vector<MeshVert>& verts = mesh.verts;
    const vector<MeshTopo>& topos = mesh.topos;
    const vector<MeshHex>& hexs = mesh.hexs;

    for(int eId = first; eId < last; ++eId)
    {
        const MeshHex& hex = hexs[eId];

        uint vi[] = {
            hex.v[0],
            hex.v[1],
            hex.v[2],
            hex.v[3],
            hex.v[4],
            hex.v[5],
            hex.v[6],
            hex.v[7],
        };

        glm::dvec3 vp[] = {
            verts[vi[0]].p,
            verts[vi[1]].p,
            verts[vi[2]].p,
            verts[vi[3]].p,
            verts[vi[4]].p,
            verts[vi[5]].p,
            verts[vi[6]].p,
            verts[vi[7]].p,
        };

        glm::dvec3 aux[] = {
            (vp[0] + vp[1] + vp[2] + vp[3]) / 4.0,
            (vp[0] + vp[4] + vp[5] + vp[1]) / 4.0,
            (vp[1] + vp[5] + vp[6] + vp[2]) / 4.0,
            (vp[2] + vp[6] + vp[7] + vp[3]) / 4.0,
            (vp[0] + vp[3] + vp[7] + vp[4]) / 4.0,
            (vp[4] + vp[7] + vp[6] + vp[5]) / 4.0,
        };

        glm::dvec3 n[] = {
            glm::cross(aux[1] - aux[0], aux[4] - aux[0]),
            glm::cross(aux[2] - aux[0], aux[1] - aux[0]),
            glm::cross(aux[3] - aux[0], aux[2] - aux[0]),
            glm::cross(aux[4] - aux[0], aux[3] - aux[0]),
            glm::cross(aux[4] - aux[5], aux[1] - aux[5]),
            glm::cross(aux[1] - aux[5], aux[2] - aux[5]),
            glm::cross(aux[2] - aux[5], aux[3] - aux[5]),
            glm::cross(aux[3] - aux[5], aux[4] - aux[5]),
        };

        glm::dvec3 bases[] = {
            (aux[0] + aux[1] + aux[4]) / 3.0,
            (aux[0] + aux[2] + aux[1]) / 3.0,
            (aux[0] + aux[3] + aux[2]) / 3.0,
            (aux[0] + aux[4] + aux[3]) / 3.0,
            (aux[5] + aux[4] + aux[1]) / 3.0,
            (aux[5] + aux[1] + aux[2]) / 3.0,
            (aux[5] + aux[2] + aux[3]) / 3.0,
            (aux[5] + aux[3] + aux[4]) / 3.0,
        };


        // New positions
        glm::dvec3 vpp[] = {
            bases[0] + n[0] * (_lambda / glm::sqrt(glm::length(n[0]))),
            bases[1] + n[1] * (_lambda / glm::sqrt(glm::length(n[1]))),
            bases[2] + n[2] * (_lambda / glm::sqrt(glm::length(n[2]))),
            bases[3] + n[3] * (_lambda / glm::sqrt(glm::length(n[3]))),
            bases[4] + n[4] * (_lambda / glm::sqrt(glm::length(n[4]))),
            bases[5] + n[5] * (_lambda / glm::sqrt(glm::length(n[5]))),
            bases[6] + n[6] * (_lambda / glm::sqrt(glm::length(n[6]))),
            bases[7] + n[7] * (_lambda / glm::sqrt(glm::length(n[7]))),
        };


        double volume = crew.measurer().hexVolume(crew.sampler(), vp, hex);
        double volumePrime = crew.measurer().hexVolume(crew.sampler(), vpp, hex);
        double absVolumeRation = glm::abs(volume / volumePrime);
        double volumeVar = glm::pow(absVolumeRation, 1.0/3.0);

        glm::dvec3 center = (1.0/8.0) * (
            vp[0] + vp[1] + vp[2] + vp[3] + vp[4] + vp[5] + vp[6] + vp[7]);

        vpp[0] = center + volumeVar * (vpp[0] - center);
        vpp[1] = center + volumeVar * (vpp[1] - center);
        vpp[2] = center + volumeVar * (vpp[2] - center);
        vpp[3] = center + volumeVar * (vpp[3] - center);
        vpp[4] = center + volumeVar * (vpp[4] - center);
        vpp[5] = center + volumeVar * (vpp[5] - center);
        vpp[6] = center + volumeVar * (vpp[6] - center);
        vpp[7] = center + volumeVar * (vpp[7] - center);

        if(topos[vi[0]].snapToBoundary->isConstrained())
            vpp[0] = (*topos[vi[0]].snapToBoundary)(vpp[0]);
        if(topos[vi[1]].snapToBoundary->isConstrained())
            vpp[1] = (*topos[vi[1]].snapToBoundary)(vpp[1]);
        if(topos[vi[2]].snapToBoundary->isConstrained())
            vpp[2] = (*topos[vi[2]].snapToBoundary)(vpp[2]);
        if(topos[vi[3]].snapToBoundary->isConstrained())
            vpp[3] = (*topos[vi[3]].snapToBoundary)(vpp[3]);
        if(topos[vi[4]].snapToBoundary->isConstrained())
            vpp[4] = (*topos[vi[4]].snapToBoundary)(vpp[4]);
        if(topos[vi[5]].snapToBoundary->isConstrained())
            vpp[5] = (*topos[vi[5]].snapToBoundary)(vpp[5]);
        if(topos[vi[6]].snapToBoundary->isConstrained())
            vpp[6] = (*topos[vi[6]].snapToBoundary)(vpp[6]);
        if(topos[vi[7]].snapToBoundary->isConstrained())
            vpp[7] = (*topos[vi[7]].snapToBoundary)(vpp[7]);


        double quality = crew.evaluator().hexQuality(crew.sampler(), crew.measurer(), vp, hex);
        double qualityPrime = crew.evaluator().hexQuality(crew.sampler(), crew.measurer(), vpp, hex);

        double weight = qualityPrime / (1.0 + quality);
        _vertexAccums[vi[0]]->addPosition(vpp[0], weight);
        _vertexAccums[vi[1]]->addPosition(vpp[1], weight);
        _vertexAccums[vi[2]]->addPosition(vpp[2], weight);
        _vertexAccums[vi[3]]->addPosition(vpp[3], weight);
        _vertexAccums[vi[4]]->addPosition(vpp[4], weight);
        _vertexAccums[vi[5]]->addPosition(vpp[5], weight);
        _vertexAccums[vi[6]]->addPosition(vpp[6], weight);
        _vertexAccums[vi[7]]->addPosition(vpp[7], weight);
    }
}
