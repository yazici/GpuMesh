#ifndef GPUMESH_LOCALNELDERMEADOPTIMISATIONSMOOTHER
#define GPUMESH_LOCALNELDERMEADOPTIMISATIONSMOOTHER


#include "AbstractVertexWiseSmoother.h"


class NelderMeadSmoother : public AbstractVertexWiseSmoother
{
public:
    NelderMeadSmoother();
    virtual ~NelderMeadSmoother();


protected:
    virtual void setVertexProgramUniforms(
            const Mesh& mesh,
            cellar::GlProgram& program);

    virtual void printSmoothingParameters(
            const Mesh& mesh,
            OptimizationPlot& plot) const override;

    virtual void smoothVertices(
            Mesh& mesh,
            const MeshCrew& crew,
            const std::vector<uint>& vIds) override;


private:
    int _securityCycleCount;
    double _localSizeToNodeShift;

    double _alpha;
    double _beta;
    double _gamma;
    double _delta;
};

#endif // GPUMESH_LOCALNELDERMEADOPTIMISATIONSMOOTHER
