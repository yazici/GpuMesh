#include "Base.cuh"

namespace nm
{
    __constant__ float VALUE_CONVERGENCE;
    __constant__ int SECURITY_CYCLE_COUNT;
    __constant__ float LOCALE_SIZE_TO_NODE_SHIFT;
    __constant__ float ALPHA;
    __constant__ float BETA;
    __constant__ float GAMMA;
    __constant__ float DELTA;
}

using namespace nm;


// Smoothing Helper
__device__ float computeLocalElementSize(uint vId);
__device__ float patchQuality(uint vId);


// ENTRY POINT //
__device__ void nelderMeadSmoothVert(uint vId)
{
    // Compute local element size
    float localSize = computeLocalElementSize(vId);

    // Initialize node shift distance
    float nodeShift = localSize * LOCALE_SIZE_TO_NODE_SHIFT;

    vec3 pos = verts[vId].p;
    vec4 vo(pos, patchQuality(vId));

    vec4 simplex[TET_VERTEX_COUNT] = {
        vec4(pos + vec3(nodeShift, 0, 0), 0),
        vec4(pos + vec3(0, nodeShift, 0), 0),
        vec4(pos + vec3(0, 0, nodeShift), 0),
        vo
    };

    int cycle = 0;
    bool reset = false;
    bool terminated = false;
    while(!terminated)
    {
        for(uint p=0; p < TET_VERTEX_COUNT-1; ++p)
        {
            // Since 'pos' is a reference on vertex's position
            // modifing its value here should be seen by the evaluator
            verts[vId].p = vec3(simplex[p]);

            // Compute patch quality
            simplex[p] = vec4(verts[vId].p, patchQuality(vId));
        }

        // Mini bubble sort
        if(simplex[0].w > simplex[1].w)
            swap(simplex[0], simplex[1]);
        if(simplex[1].w > simplex[2].w)
            swap(simplex[1], simplex[2]);
        if(simplex[2].w > simplex[3].w)
            swap(simplex[2], simplex[3]);
        if(simplex[0].w > simplex[1].w)
            swap(simplex[0], simplex[1]);
        if(simplex[1].w > simplex[2].w)
            swap(simplex[1], simplex[2]);
        if(simplex[0].w > simplex[1].w)
            swap(simplex[0], simplex[1]);


        for(; cycle < SECURITY_CYCLE_COUNT; ++cycle)
        {
            // Centroid
            vec3 c = 1/3.0f * (
                vec3(simplex[1]) +
                vec3(simplex[2]) +
                vec3(simplex[3]));

            float f = 0.0;

            // Reflect
            verts[vId].p = c + ALPHA*(c - vec3(simplex[0]));
            float fr = f = patchQuality(vId);

            vec3 xr = verts[vId].p;

            // Expand
            if(simplex[3].w < fr)
            {
                verts[vId].p = c + GAMMA*(verts[vId].p - c);
                float fe = f = patchQuality(vId);

                if(fe <= fr)
                {
                    verts[vId].p = xr;
                    f = fr;
                }
            }
            // Contract
            else if(simplex[1].w >= fr)
            {
                // Outside
                if(fr > simplex[0].w)
                {
                    verts[vId].p = c + BETA*(xr - c);
                    f = patchQuality(vId);
                }
                // Inside
                else
                {
                    verts[vId].p = c + BETA*(vec3(simplex[0]) - c), 0;
                    f = patchQuality(vId);
                }
            }

            // Insert new vertex in the working simplex
            vec4 vertex(verts[vId].p, f);
            if(vertex.w > simplex[3].w)
                swap(simplex[3], vertex);
            if(vertex.w > simplex[2].w)
                swap(simplex[2], vertex);
            if(vertex.w > simplex[1].w)
                swap(simplex[1], vertex);
            if(vertex.w > simplex[0].w)
                swap(simplex[0], vertex);


            if( (simplex[3].w - simplex[1].w) < VALUE_CONVERGENCE )
            {
                terminated = true;
                break;
            }
        }

        if( terminated || (cycle >= SECURITY_CYCLE_COUNT && reset) )
        {
            break;
        }
        else
        {
            simplex[0] = vo - vec4(nodeShift, 0, 0, 0);
            simplex[1] = vo - vec4(0, nodeShift, 0, 0);
            simplex[2] = vo - vec4(0, 0, nodeShift, 0);
            simplex[3] = vo;
            reset = true;
            cycle = 0;
        }
    }

    verts[vId].p = vec3(simplex[3]);
}

__device__ smoothVertFct nelderMeadSmoothVertPtr = nelderMeadSmoothVert;


// CUDA Drivers
void installCudaNelderMeadSmoother(
        float h_valueConvergence,
        int h_securityCycleCount,
        float h_localSizeToNodeShift,
        float h_alpha,
        float h_beta,
        float h_gamma,
        float h_delta)
{
    smoothVertFct d_smoothVert = nullptr;
    cudaMemcpyFromSymbol(&d_smoothVert, nelderMeadSmoothVertPtr, sizeof(smoothVertFct));
    cudaMemcpyToSymbol(smoothVert, &d_smoothVert, sizeof(smoothVertFct));

    cudaMemcpyToSymbol(VALUE_CONVERGENCE, &h_valueConvergence, sizeof(float));
    cudaMemcpyToSymbol(SECURITY_CYCLE_COUNT, &h_securityCycleCount, sizeof(int));
    cudaMemcpyToSymbol(LOCALE_SIZE_TO_NODE_SHIFT, &h_localSizeToNodeShift, sizeof(float));
    cudaMemcpyToSymbol(ALPHA, &h_alpha, sizeof(float));
    cudaMemcpyToSymbol(BETA,  &h_beta,  sizeof(float));
    cudaMemcpyToSymbol(GAMMA, &h_gamma, sizeof(float));
    cudaMemcpyToSymbol(DELTA, &h_delta, sizeof(float));


    if(verboseCuda)
        printf("I -> CUDA \tNelder Mead smoother installed\n");
}
