#ifndef GPUMESH_GETMESMOOTHER
#define GPUMESH_GETMESMOOTHER


#include "AbstractElementWiseSmoother.h"



class GetmeSmoother : public AbstractElementWiseSmoother
{
public:
    GetmeSmoother();
    virtual ~GetmeSmoother();

protected:

    virtual void setElementProgramUniforms(
            const Mesh& mesh,
            cellar::GlProgram& program);

    virtual void setVertexProgramUniforms(
            const Mesh& mesh,
            cellar::GlProgram& program);

    virtual void printOptimisationParameters(
            const Mesh& mesh,
            OptimizationImpl& plotImpl) const override;


    virtual void smoothTets(
            Mesh& mesh,
            const MeshCrew& crew,
            size_t first,
            size_t last) override;

    virtual void smoothPris(
            Mesh& mesh,
            const MeshCrew& crew,
            size_t first,
            size_t last) override;

    virtual void smoothHexs(
            Mesh& mesh,
            const MeshCrew& crew,
            size_t first,
            size_t last) override;


private:
    double _lambda;
};

#endif // GPUMESH_GETMESMOOTHER
