#ifndef GPUMESH_PATCHGRADDSNTSMOOTHER
#define GPUMESH_PATCHGRADDSNTSMOOTHER

#include "GradientDescentSmoother.h"

class PatchGradDsntSmoother : public GradientDescentSmoother
{
public:
    PatchGradDsntSmoother();
    virtual ~PatchGradDsntSmoother();

    virtual void smoothMeshGlsl(
            Mesh& mesh,
            const MeshCrew& crew) override;

    virtual void smoothMeshCuda(
            Mesh& mesh,
            const MeshCrew& crew) override;

protected:
    virtual bool verifyMeshForGpuLimitations(
            const Mesh& mesh) const;

    virtual std::string glslLauncher() const override;

    virtual NodeGroups::GpuDispatcher glslDispatcher() const override;

    virtual NodeGroups::GpuDispatcher cudaDispatcher() const override;

protected:
    static const int POSITION_THREAD_COUNT;
    static const int ELEMENT_THREAD_COUNT;

    static const int ELEMENT_PER_THREAD_COUNT;
};

#endif // GPUMESH_PATCHGRADDSNTSMOOTHER
