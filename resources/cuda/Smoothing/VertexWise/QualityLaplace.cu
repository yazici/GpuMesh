#include "Base.cuh"

#define PROPOSITION_COUNT uint(4)

__device__  float QLMoveCoeff = 0.35;


// Smoothing helper
__device__ float patchQuality(uint vId);


// ENTRY POINT //
__device__ void qualityLaplaceSmoothVert(uint vId)
{
    // Compute patch center
    vec3 pos = verts[vId].p;
    vec3 patchCenter = computeVertexEquilibrium(vId);
    vec3 centerDist = patchCenter - pos;


    // Define propositions for new vertex's position
    vec3 propositions[PROPOSITION_COUNT] = {
        pos,
        patchCenter - centerDist * QLMoveCoeff,
        patchCenter,
        patchCenter + centerDist * QLMoveCoeff
    };


    // Choose best position based on quality geometric mean
    uint bestProposition = 0;
    float bestQualityMean = -1.0/0.0; // -Inf
    for(uint p=0; p < PROPOSITION_COUNT; ++p)
    {
        // Quality evaluation functions will use this updated position
        // to compute element shape measures.
        verts[vId].p = propositions[p];

        float pq = patchQuality(vId);

        if(pq > bestQualityMean)
        {
            bestQualityMean = pq;
            bestProposition = p;
        }
    }


    // Update vertex's position
    verts[vId].p = propositions[bestProposition];
}

__device__ smoothVertFct qualityLaplaceSmoothVertPtr = qualityLaplaceSmoothVert;


// CUDA Drivers
void installCudaQualityLaplaceSmoother(float moveCoeff)
{
    cudaMemcpyToSymbol(QLMoveCoeff, &moveCoeff, sizeof(float));

    smoothVertFct d_smoothVert = nullptr;
    cudaMemcpyFromSymbol(&d_smoothVert, qualityLaplaceSmoothVertPtr, sizeof(smoothVertFct));
    cudaMemcpyToSymbol(smoothVert, &d_smoothVert, sizeof(smoothVertFct));


    if(verboseCuda)
        printf("I -> CUDA \tQuality Laplace smoother installed\n");
}
