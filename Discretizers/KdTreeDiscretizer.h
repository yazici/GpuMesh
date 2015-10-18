#ifndef GPUMESH_KDTREEDISCRETIZER
#define GPUMESH_KDTREEDISCRETIZER

#include <vector>

#include "AbstractDiscretizer.h"

struct KdNode;


class KdTreeDiscretizer : public AbstractDiscretizer
{
public:
    KdTreeDiscretizer();
    virtual ~KdTreeDiscretizer();


    virtual bool isMetricWise() const;


    virtual void installPlugIn(
            const Mesh& mesh,
            cellar::GlProgram& program) const override;

    virtual void uploadUniforms(
            const Mesh& mesh,
            cellar::GlProgram& program) const override;


    virtual void discretize(
            const Mesh& mesh,
            int density) override;

    virtual Metric metric(
            const glm::dvec3& position) const override;


    virtual void releaseDebugMesh() override;
    virtual const Mesh& debugMesh() override;


private:
    void build(
            KdNode* node,
            int height,
            const Mesh& mesh,
            const glm::dvec3& minBox,
            const glm::dvec3& maxBox,
            std::vector<uint>& xSort,
            std::vector<uint>& ySort,
            std::vector<uint>& zSort);

    void meshTree(KdNode* node, Mesh& mesh);

    std::unique_ptr<KdNode> _rootNode;
    std::shared_ptr<Mesh> _debugMesh;
};

#endif // GPUMESH_KDTREEDISCRETIZER
