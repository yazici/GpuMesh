#include "Base.cuh"


// Distance
__device__ float metricFreeRiemannianDistance(const vec3& a, const vec3& b)
{
    return distance(a, b);
}

__device__ vec3 metricFreeRiemannianSegment(const vec3& a, const vec3& b)
{
    return b - a;
}


// Volume
__device__ float metricFreeTetVolume(const vec3 vp[TET_VERTEX_COUNT])
{
    float detSum = 0.0;
    detSum += determinant(mat3(
        vp[0] - vp[3],
        vp[1] - vp[3],
        vp[2] - vp[3]));

    return detSum / 6.0;
}

__device__ float metricFreePriVolume(const vec3 vp[PRI_VERTEX_COUNT])
{
    float detSum = 0.0;
    detSum += determinant(mat3(
        vp[4] - vp[2],
        vp[0] - vp[2],
        vp[1] - vp[2]));
    detSum += determinant(mat3(
        vp[5] - vp[2],
        vp[1] - vp[2],
        vp[3] - vp[2]));
    detSum += determinant(mat3(
        vp[4] - vp[2],
        vp[1] - vp[2],
        vp[5] - vp[2]));

    return detSum / 6.0;
}

__device__ float metricFreeHexVolume(const vec3 vp[HEX_VERTEX_COUNT])
{
    float detSum = 0.0;
    detSum += determinant(mat3(
        vp[0] - vp[2],
        vp[1] - vp[2],
        vp[4] - vp[2]));
    detSum += determinant(mat3(
        vp[3] - vp[1],
        vp[2] - vp[1],
        vp[7] - vp[1]));
    detSum += determinant(mat3(
        vp[5] - vp[4],
        vp[1] - vp[4],
        vp[7] - vp[4]));
    detSum += determinant(mat3(
        vp[6] - vp[7],
        vp[2] - vp[7],
        vp[4] - vp[7]));
    detSum += determinant(mat3(
        vp[1] - vp[2],
        vp[7] - vp[2],
        vp[4] - vp[2]));

    return detSum / 6.0;
}


// High level measurement
__device__ vec3 metricFreeComputeVertexEquilibrium(uint vId)
{
    Topo topo = topos[vId];

    uint totalVertCount = 0;
    vec3 patchCenter = vec3(0.0);
    uint neigElemCount = topo.neigElemCount;
    for(uint i=0, n = topo.neigElemBase; i<neigElemCount; ++i, ++n)
    {
        NeigElem neigElem = neigElems[n];

        switch(neigElem.type)
        {
        case TET_ELEMENT_TYPE:
            totalVertCount += TET_VERTEX_COUNT - 1;
            for(uint i=0; i < TET_VERTEX_COUNT; ++i)
                patchCenter += vec3(verts[tets[neigElem.id].v[i]].p);
            break;

        case PRI_ELEMENT_TYPE:
            totalVertCount += PRI_VERTEX_COUNT - 1;
            for(uint i=0; i < PRI_VERTEX_COUNT; ++i)
                patchCenter += vec3(verts[pris[neigElem.id].v[i]].p);
            break;

        case HEX_ELEMENT_TYPE:
            totalVertCount += HEX_VERTEX_COUNT - 1;
            for(uint i=0; i < HEX_VERTEX_COUNT; ++i)
                patchCenter += vec3(verts[hexs[neigElem.id].v[i]].p);
            break;
        }
    }

    vec3 pos = vec3(verts[vId].p);
    patchCenter = (patchCenter - pos * float(neigElemCount))
                    / float(totalVertCount);
    return patchCenter;
}


// Fonction pointers
__device__ riemannianDistanceFct metricFreeRiemannianDistancePtr = metricFreeRiemannianDistance;
__device__ riemannianSegmentFct metricFreeRiemannianSegmentPtr = metricFreeRiemannianSegment;

__device__ tetVolumeFct metricFreeTetVolumePtr = metricFreeTetVolume;
__device__ priVolumeFct metricFreePriVolumePtr = metricFreePriVolume;
__device__ hexVolumeFct metricFreeHexVolumePtr = metricFreeHexVolume;

__device__ computeVertexEquilibriumFct metricFreeComputeVertexEquilibriumPtr = metricFreeComputeVertexEquilibrium;


// CUDA Drivers
void installCudaMetricFreeMeasurer()
{
    riemannianDistanceFct d_riemannianDistance = nullptr;
    cudaMemcpyFromSymbol(&d_riemannianDistance, metricFreeRiemannianDistancePtr, sizeof(riemannianDistanceFct));
    cudaMemcpyToSymbol(riemannianDistanceImpl, &d_riemannianDistance, sizeof(riemannianDistanceFct));

    riemannianSegmentFct d_riemannianSegment = nullptr;
    cudaMemcpyFromSymbol(&d_riemannianSegment, metricFreeRiemannianSegmentPtr, sizeof(riemannianSegmentFct));
    cudaMemcpyToSymbol(riemannianSegmentImpl, &d_riemannianSegment, sizeof(riemannianSegmentFct));


    tetVolumeFct d_tetVolume = nullptr;
    cudaMemcpyFromSymbol(&d_tetVolume, metricFreeTetVolumePtr, sizeof(tetVolumeFct));
    cudaMemcpyToSymbol(tetVolumeImpl, &d_tetVolume, sizeof(tetVolumeFct));

    priVolumeFct d_priVolume = nullptr;
    cudaMemcpyFromSymbol(&d_priVolume, metricFreePriVolumePtr, sizeof(priVolumeFct));
    cudaMemcpyToSymbol(priVolumeImpl, &d_priVolume, sizeof(priVolumeFct));

    hexVolumeFct d_hexVolume = nullptr;
    cudaMemcpyFromSymbol(&d_hexVolume, metricFreeHexVolumePtr, sizeof(hexVolumeFct));
    cudaMemcpyToSymbol(hexVolumeImpl, &d_hexVolume, sizeof(hexVolumeFct));


    computeVertexEquilibriumFct d_computeVertexEquilibrium = nullptr;
    cudaMemcpyFromSymbol(&d_computeVertexEquilibrium, metricFreeComputeVertexEquilibriumPtr, sizeof(computeVertexEquilibriumFct));
    cudaMemcpyToSymbol(computeVertexEquilibrium, &d_computeVertexEquilibrium, sizeof(computeVertexEquilibriumFct));


    printf("I -> CUDA \tMetric Free Measurer installed\n");
}
