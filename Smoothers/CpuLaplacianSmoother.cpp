#include "CpuLaplacianSmoother.h"

#include <iostream>

using namespace std;


CpuLaplacianSmoother::CpuLaplacianSmoother(
        double moveFactor,
        double gainThreshold) :
    AbstractSmoother(moveFactor, gainThreshold)
{

}

CpuLaplacianSmoother::~CpuLaplacianSmoother()
{

}

void CpuLaplacianSmoother::smoothMesh(Mesh& mesh, AbstractEvaluator& evaluator)
{
    evaluateInitialMeshQuality(mesh, evaluator);

    while(evaluateIterationMeshQuality(mesh, evaluator))
    {

        int vertCount = mesh.vert.size();
        for(int v = 0; v < vertCount; ++v)
        {
            glm::dvec3& pos = mesh.vert[v].p;
            const MeshTopo& topo = mesh.topo[v];
            if(topo.isFixed)
                continue;

            const vector<int>& neighbors = topo.neighbors;
            if(!neighbors.empty())
            {
                double weightSum = 0.0;
                glm::dvec3 barycenter(0.0);

                int neighborCount = neighbors.size();
                for(int i=0; i<neighborCount; ++i)
                {
                    glm::dvec3 npos(mesh.vert[neighbors[i]]);

                    glm::dvec3 dist = npos - pos;
                    double weight = glm::dot(dist, dist) + 0.0001;

                    barycenter += npos * weight;
                    weightSum += weight;
                }

                barycenter /= weightSum;
                pos = glm::mix(pos, barycenter, _moveFactor);

                if(topo.isBoundary)
                {
                    pos = topo.boundaryCallback(pos);
                }
            }
        }
    }

    cout << "#Smoothing finished" << endl << endl;
}
