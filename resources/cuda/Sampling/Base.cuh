#include "../Mesh.cuh"


// Metric sampling function
typedef mat3 (*metricAtFct)(const vec3&, uint&);
extern __device__ metricAtFct metricAt;


// Metric Scaling
extern __constant__ float MetricScaling;
