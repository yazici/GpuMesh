#ifndef GPUMESH_ABSTRACTEVALUATOR
#define GPUMESH_ABSTRACTEVALUATOR

#include <CellarWorkbench/GL/GlProgram.h>

#include "DataStructures/Mesh.h"


class AbstractEvaluator
{
public:
    AbstractEvaluator(const std::string& shapeMeasuresShader);
    virtual ~AbstractEvaluator();


    virtual double tetrahedronQuality(const Mesh& mesh, const MeshTet& tet) const = 0;

    virtual double prismQuality(const Mesh& mesh, const MeshPri& pri) const = 0;

    virtual double hexahedronQuality(const Mesh& mesh, const MeshHex& hex) const = 0;

    virtual bool assessMeasureValidy();

    virtual void evaluateCpuMeshQuality(
            const Mesh& mesh,
            double& minQuality,
            double& qualityMean);

    virtual void evaluateGpuMeshQuality(
            const Mesh& mesh,
            double& minQuality,
            double& qualityMean);


protected:
    virtual void initializeProgram(const Mesh& mesh);

    bool _initialized;
    bool _computeSimultaneously;
    std::string _shapeMeasuresShader;
    cellar::GlProgram _simultaneousProgram;
    cellar::GlProgram _tetProgram;
    cellar::GlProgram _priProgram;
    cellar::GlProgram _hexProgram;
    GLuint _qualSsbo;

    static const double VALIDITY_EPSILON;
    static const double MAX_INTEGER_VALUE;
    static const double MIN_QUALITY_PRECISION_DENOM;
};

#endif // GPUMESH_ABSTRACTEVALUATOR
