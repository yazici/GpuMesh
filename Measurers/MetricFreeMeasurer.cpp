#include "MetricFreeMeasurer.h"

#include "DataStructures/Mesh.h"
#include "Evaluators/AbstractEvaluator.h"

using namespace std;


MetricFreeMeasurer::MetricFreeMeasurer() :
    AbstractMeasurer("Metric Free", ":/shaders/compute/Measuring/MetricFree.glsl")
{

}

MetricFreeMeasurer::~MetricFreeMeasurer()
{

}

double MetricFreeMeasurer::measuredDistance(
        const AbstractDiscretizer& discretizer,
        const glm::dvec3& a,
        const glm::dvec3& b) const
{
    return glm::distance(a, b);
}

double MetricFreeMeasurer::computeLocalElementSize(
        const Mesh& mesh,
        size_t vId) const
{
    const std::vector<MeshVert>& verts = mesh.verts;

    const glm::dvec3& pos = verts[vId].p;
    const MeshTopo& topo = mesh.topos[vId];
    const vector<MeshNeigVert>& neigVerts = topo.neighborVerts;

    double totalSize = 0.0;
    size_t neigVertCount = neigVerts.size();
    for(size_t n=0; n < neigVertCount; ++n)
    {
        totalSize += glm::length(pos - verts[neigVerts[n].v].p);
    }

    return totalSize / neigVertCount;
}

glm::dvec3 MetricFreeMeasurer::computeVertexEquilibrium(
        const Mesh& mesh,
        const AbstractDiscretizer& discretizer,
        size_t vId) const
{
    const std::vector<MeshVert>& verts = mesh.verts;
    const std::vector<MeshTet>& tets = mesh.tets;
    const std::vector<MeshPri>& pris = mesh.pris;
    const std::vector<MeshHex>& hexs = mesh.hexs;

    const MeshTopo& topo = mesh.topos[vId];

    uint totalVertCount = 0;
    glm::dvec3 patchCenter(0.0);
    uint neigElemCount = topo.neighborElems.size();
    for(uint n=0; n < neigElemCount; ++n)
    {
        const MeshNeigElem& neigElem = topo.neighborElems[n];
        switch(neigElem.type)
        {
        case MeshTet::ELEMENT_TYPE:
            totalVertCount += MeshTet::VERTEX_COUNT - 1;
            for(uint i=0; i < MeshTet::VERTEX_COUNT; ++i)
                patchCenter += verts[tets[neigElem.id].v[i]].p;
            break;

        case MeshPri::ELEMENT_TYPE:
            totalVertCount += MeshPri::VERTEX_COUNT - 1;
            for(uint i=0; i < MeshPri::VERTEX_COUNT; ++i)
                patchCenter += verts[pris[neigElem.id].v[i]].p;
            break;

        case MeshHex::ELEMENT_TYPE:
            totalVertCount += MeshHex::VERTEX_COUNT - 1;
            for(uint i=0; i < MeshHex::VERTEX_COUNT; ++i)
                patchCenter += verts[hexs[neigElem.id].v[i]].p;
            break;
        }
    }

    const glm::dvec3& pos = verts[vId].p;
    patchCenter = (patchCenter - pos * double(neigElemCount))
                    / double(totalVertCount);

    return patchCenter;
}

double MetricFreeMeasurer::computePatchQuality(
            const Mesh& mesh,
            const AbstractEvaluator& evaluator,
            size_t vId) const
{
    const std::vector<MeshTet>& tets = mesh.tets;
    const std::vector<MeshPri>& pris = mesh.pris;
    const std::vector<MeshHex>& hexs = mesh.hexs;

    const MeshTopo& topo = mesh.topos[vId];

    size_t neigElemCount = topo.neighborElems.size();

    double patchWeight = 0.0;
    double patchQuality = 1.0;
    for(size_t n=0; n < neigElemCount; ++n)
    {
        const MeshNeigElem& neigElem = topo.neighborElems[n];

        switch(neigElem.type)
        {
        case MeshTet::ELEMENT_TYPE:
            accumulatePatchQuality(
                patchQuality, patchWeight,
                evaluator.tetQuality(mesh, tets[neigElem.id]));
            break;

        case MeshPri::ELEMENT_TYPE:
            accumulatePatchQuality(
                patchQuality, patchWeight,
                evaluator.priQuality(mesh, pris[neigElem.id]));
            break;

        case MeshHex::ELEMENT_TYPE:
            accumulatePatchQuality(
                patchQuality, patchWeight,
                evaluator.hexQuality(mesh, hexs[neigElem.id]));
            break;
        }
    }

    return finalizePatchQuality(patchQuality, patchWeight);
}
