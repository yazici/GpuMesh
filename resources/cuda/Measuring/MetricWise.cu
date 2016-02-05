#include "Base.cuh"


// Distance
__device__ float metricWiseRiemannianDistance(const vec3& a, const vec3& b)
{
    vec3 abDiff = b - a;
    vec3 middle = (a + b) / 2.0f;
    float dist = sqrt(dot(abDiff, metricAt(middle) * abDiff));

    int segmentCount = 1;
    float err = 1.0;
    while(err > 1e-4)
    {
        segmentCount *= 2;
        vec3 segBeg = a;
        vec3 ds = abDiff / float(segmentCount);
        vec3 half_ds = ds / 2.0f;

        float newDist = 0.0;
        for(int i=0; i < segmentCount; ++i)
        {
            mat3 metric = metricAt(segBeg + half_ds);
            newDist += sqrt(dot(ds, metric * ds));
            segBeg += ds;
        }

        err = abs(newDist - dist);
        dist = newDist;
    }

    return dist;
}

__device__ vec3 metricWiseRiemannianSegment(const vec3& a, const vec3& b)
{
    return normalize(b - a) *
        metricWiseRiemannianDistance(a, b);
}


// Volume
__device__ float metricWiseTetVolume(const vec3 vp[TET_VERTEX_COUNT])
{
    float detSum = determinant(mat3(
        metricWiseRiemannianSegment(vp[3], vp[0]),
        metricWiseRiemannianSegment(vp[3], vp[1]),
        metricWiseRiemannianSegment(vp[3], vp[2])));

    return detSum / 6.0;
}

__device__ float metricWisePriVolume(const vec3 vp[PRI_VERTEX_COUNT])
{
    vec3 e20 = metricWiseRiemannianSegment(vp[2], vp[0]);
    vec3 e21 = metricWiseRiemannianSegment(vp[2], vp[1]);
    vec3 e23 = metricWiseRiemannianSegment(vp[2], vp[3]);
    vec3 e24 = metricWiseRiemannianSegment(vp[2], vp[4]);
    vec3 e25 = metricWiseRiemannianSegment(vp[2], vp[5]);

    float detSum = 0.0;
    detSum += determinant(mat3(e24, e20, e21));
    detSum += determinant(mat3(e25, e21, e23));
    detSum += determinant(mat3(e24, e21, e25));

    return detSum / 6.0;
}

__device__ float metricWiseHexVolume(const vec3 vp[HEX_VERTEX_COUNT])
{
    float detSum = 0.0;
    detSum += determinant(mat3(
        metricWiseRiemannianSegment(vp[0], vp[1]),
        metricWiseRiemannianSegment(vp[0], vp[2]),
        metricWiseRiemannianSegment(vp[0], vp[4])));
    detSum += determinant(mat3(
        metricWiseRiemannianSegment(vp[3], vp[1]),
        metricWiseRiemannianSegment(vp[3], vp[7]),
        metricWiseRiemannianSegment(vp[3], vp[2])));
    detSum += determinant(mat3(
        metricWiseRiemannianSegment(vp[5], vp[1]),
        metricWiseRiemannianSegment(vp[5], vp[4]),
        metricWiseRiemannianSegment(vp[5], vp[7])));
    detSum += determinant(mat3(
        metricWiseRiemannianSegment(vp[6], vp[2]),
        metricWiseRiemannianSegment(vp[6], vp[7]),
        metricWiseRiemannianSegment(vp[6], vp[4])));
    detSum += determinant(mat3(
        metricWiseRiemannianSegment(vp[1], vp[2]),
        metricWiseRiemannianSegment(vp[1], vp[4]),
        metricWiseRiemannianSegment(vp[1], vp[7])));

    return detSum / 6.0;
}


// High level measurement
__device__ vec3 computeSpringForce(const vec3& pi, const vec3& pj)
{
    if(pi == pj)
        return vec3(0.0);

    float d = metricWiseRiemannianDistance(pi, pj);
    vec3 u = (pi - pj) / d;

    float d2 = d * d;
    //float d4 = d2 * d2;

    //float f = (1 - d4) * exp(-d4);
    //float f = (1-d2)*exp(-d2/4.0)/2.0;
    float f = (1-d2)*exp(-abs(d)/(sqrt(2.0)));

    return f * u;
}

__device__ vec3 metricWiseComputeVertexEquilibrium(uint vId)
{
    Topo topo = topos[vId];
    vec3 pos = vec3(verts[vId].p);

    vec3 forceTotal = vec3(0.0);
    uint neigVertCount = topo.neigVertCount;
    for(uint i=0, n = topo.neigVertBase; i < neigVertCount; ++i, ++n)
    {
        NeigVert neigVert = neigVerts[n];
        vec3 npos = vec3(verts[neigVert.v].p);

        forceTotal += computeSpringForce(pos, npos);
    }

    vec3 equilibrium = pos + forceTotal;
    return equilibrium;
}


// Fonction pointers
__device__ riemannianDistanceFct metricWiseRiemannianDistancePtr = metricWiseRiemannianDistance;
__device__ riemannianSegmentFct metricWiseRiemannianSegmentPtr = metricWiseRiemannianSegment;

__device__ tetVolumeFct metricWiseTetVolumePtr = metricWiseTetVolume;
__device__ priVolumeFct metricWisePriVolumePtr = metricWisePriVolume;
__device__ hexVolumeFct metricWiseHexVolumePtr = metricWiseHexVolume;

__device__ computeVertexEquilibriumFct metricWiseComputeVertexEquilibriumPtr = metricWiseComputeVertexEquilibrium;


// CUDA Drivers
void installCudaMetricWiseMeasurer()
{
    riemannianDistanceFct d_riemannianDistance = nullptr;
    cudaMemcpyFromSymbol(&d_riemannianDistance, metricWiseRiemannianDistancePtr, sizeof(riemannianDistanceFct));
    cudaMemcpyToSymbol(riemannianDistanceImpl, &d_riemannianDistance, sizeof(riemannianDistanceFct));

    riemannianSegmentFct d_riemannianSegment = nullptr;
    cudaMemcpyFromSymbol(&d_riemannianSegment, metricWiseRiemannianSegmentPtr, sizeof(riemannianSegmentFct));
    cudaMemcpyToSymbol(riemannianSegmentImpl, &d_riemannianSegment, sizeof(riemannianSegmentFct));


    tetVolumeFct d_tetVolume = nullptr;
    cudaMemcpyFromSymbol(&d_tetVolume, metricWiseTetVolumePtr, sizeof(tetVolumeFct));
    cudaMemcpyToSymbol(tetVolumeImpl, &d_tetVolume, sizeof(tetVolumeFct));

    priVolumeFct d_priVolume = nullptr;
    cudaMemcpyFromSymbol(&d_priVolume, metricWisePriVolumePtr, sizeof(priVolumeFct));
    cudaMemcpyToSymbol(priVolumeImpl, &d_priVolume, sizeof(priVolumeFct));

    hexVolumeFct d_hexVolume = nullptr;
    cudaMemcpyFromSymbol(&d_hexVolume, metricWiseHexVolumePtr, sizeof(hexVolumeFct));
    cudaMemcpyToSymbol(hexVolumeImpl, &d_hexVolume, sizeof(hexVolumeFct));


    computeVertexEquilibriumFct d_computeVertexEquilibrium = nullptr;
    cudaMemcpyFromSymbol(&d_computeVertexEquilibrium, metricWiseComputeVertexEquilibriumPtr, sizeof(computeVertexEquilibriumFct));
    cudaMemcpyToSymbol(computeVertexEquilibrium, &d_computeVertexEquilibrium, sizeof(computeVertexEquilibriumFct));


    printf("I -> CUDA \tMetric Wise Measurer installed\n");
}
