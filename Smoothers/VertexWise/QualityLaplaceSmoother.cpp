#include "QualityLaplaceSmoother.h"

#include "DataStructures/MeshCrew.h"
#include "Evaluators/AbstractEvaluator.h"
#include "Measurers/AbstractMeasurer.h"

using namespace std;


const uint PROPOSITION_COUNT = 4;


// CUDA Drivers
void installCudaQualityLaplaceSmoother();


QualityLaplaceSmoother::QualityLaplaceSmoother() :
    AbstractVertexWiseSmoother(
        {":/glsl/compute/Smoothing/VertexWise/QualityLaplace.glsl"},
        installCudaQualityLaplaceSmoother)
{

}

QualityLaplaceSmoother::~QualityLaplaceSmoother()
{

}

void QualityLaplaceSmoother::printOptimisationParameters(
        const Mesh& mesh,
        OptimizationPlot& plot) const
{
    AbstractVertexWiseSmoother::printOptimisationParameters(mesh, plot);
    plot.addSmoothingProperty("Method Name", "Quality Laplace");
    plot.addSmoothingProperty("Line Sample Count", to_string(PROPOSITION_COUNT));
    plot.addSmoothingProperty("Line Gaps", to_string(_moveCoeff));
}

void QualityLaplaceSmoother::smoothVertices(
        Mesh& mesh,
        const MeshCrew& crew,
        const std::vector<uint>& vIds)
{
    std::vector<MeshVert>& verts = mesh.verts;
    const vector<MeshTopo>& topos = mesh.topos;

    size_t vIdCount = vIds.size();
    for(int v = 0; v < vIdCount; ++v)
    {
        uint vId = vIds[v];

        if(!isSmoothable(mesh, vId))
            continue;


        // Compute patch center
        glm::dvec3 patchCenter =
            crew.measurer().computeVertexEquilibrium(
                mesh, crew.sampler(), vId);

        glm::dvec3& pos = verts[vId].p;
        glm::dvec3 centerDist = patchCenter - pos;


        // Define propositions for new vertex's position
        glm::dvec3 propositions[PROPOSITION_COUNT] = {
            pos,
            patchCenter - centerDist * _moveCoeff,
            patchCenter,
            patchCenter + centerDist * _moveCoeff,
        };

        const MeshTopo& topo = topos[vId];
        if(topo.isBoundary)
        {
            for(uint p=1; p < PROPOSITION_COUNT; ++p)
                propositions[p] = (*topo.snapToBoundary)(propositions[p]);
        }


        // Choose best position based on quality geometric mean
        uint bestProposition = 0;
        double bestQualityMean = 0.0;
        for(uint p=0; p < PROPOSITION_COUNT; ++p)
        {
            // Since 'pos' is a reference on vertex's position
            // modifing its value here should be seen by the evaluator
            pos = propositions[p];

            // Compute patch quality
            double patchQuality =
                crew.evaluator().patchQuality(
                    mesh, crew.sampler(), crew.measurer(), vId);

            if(patchQuality > bestQualityMean)
            {
                bestQualityMean = patchQuality;
                bestProposition = p;
            }
        }


        // Update vertex's position
        pos = propositions[bestProposition];
    }
}
