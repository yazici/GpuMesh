#ifndef GPUMESH_ABSTRACTRENDERER
#define GPUMESH_ABSTRACTRENDERER

#include <map>
#include <functional>

#include <CellarWorkbench/Camera/Camera.h>
#include <CellarWorkbench/DesignPattern/SpecificObserver.h>

#include <Scaena/Play/Play.h>

#include "DataStructures/Mesh.h"

class AbstractEvaluator;


class AbstractRenderer:
        public cellar::SpecificObserver<cellar::CameraMsg>
{
public:
    AbstractRenderer();
    virtual ~AbstractRenderer();

    virtual void setup();
    virtual void tearDown();
    virtual void notifyMeshUpdate();
    virtual void display(const Mesh& mesh, const AbstractEvaluator& evaluator);

    virtual std::vector<std::string> availableShadings() const;
    virtual void useShading(const std::string& shadingName);
    virtual void useVirtualCutPlane(bool use);


    virtual void updateCamera(const glm::mat4& view,
                              const glm::vec3& pos) = 0;
    virtual void updateLight(const glm::mat4& view,
                             const glm::vec3& pos) = 0;
    virtual void updateCutPlane(const glm::dvec4& cutEq) = 0;

    virtual void handleKeyPress(const scaena::KeyboardEvent& event) = 0;
    virtual void handleInputs(const scaena::SynchronousKeyboard& keyboard,
                              const scaena::SynchronousMouse& mouse) = 0;


protected:
    virtual void updateGeometry(
            const Mesh& mesh,
            const AbstractEvaluator& evaluator) = 0;

    virtual void clearResources() = 0;
    virtual void resetResources() = 0;
    virtual void setupShaders() = 0;
    virtual void render() = 0;

    bool _buffNeedUpdate;
    bool _isPhysicalCut;
    glm::dvec4 _cutPlane;

    // Shadings
    typedef std::function<void(void)> ShadingFunc;
    std::map<std::string, ShadingFunc> _shadingFuncs;
};

#endif // GPUMESH_ABSTRACTRENDERER
