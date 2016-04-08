#ifndef GPUMESH_BARTTOPOLOGIST
#define GPUMESH_BARTTOPOLOGIST

#include "AbstractTopologist.h"


class BatrTopologist : public AbstractTopologist
{
public:
    BatrTopologist();

    virtual ~BatrTopologist();

    virtual void restructureMesh(
            Mesh& mesh,
            const MeshCrew& crew) const override;

    virtual void printOptimisationParameters(
            const Mesh& mesh,
            OptimizationPlot& plot) const override;


protected:
    virtual void edgeSplitting(
            Mesh& mesh,
            const MeshCrew& crew) const;

    virtual void faceSwapping(
            Mesh& mesh,
            const MeshCrew& crew) const;

    virtual void edgeSwapping(
            Mesh& mesh,
            const MeshCrew& crew) const;
};

#endif // GPUMESH_BARTTOPOLOGIST
