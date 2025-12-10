#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>
#include <maya/MGlobal.h>
#include <maya/MPxLocatorNode.h>
#include "deltamush.h"
using namespace MHWRender;

// MyLocator node is drawn using MyLocatorDrawOverride
class MyLocator : public MPxLocatorNode
{
public:
    static MTypeId id;
    static MObject locatorMsgAttr;
    static void* creator() { return new MyLocator(); }
    static MStatus initialize();
	std::shared_ptr<DeltaMush> deltamush;
    MStatus connectionMade(const MPlug& plug, const MPlug& otherPlug, bool asSrc) override;

};

class MyLocatorDrawOverride : public MPxDrawOverride
{
public:
    static MPxDrawOverride* Creator(const MObject& obj)
    {
        return new MyLocatorDrawOverride(obj);
    }

    MyLocatorDrawOverride(const MObject& obj)
        : MPxDrawOverride(obj, nullptr, /* isAlwaysDirty */ true) {
    }

    ~MyLocatorDrawOverride() override {}


    bool traceCallSequence() const override {
        return true; // Return true to enable the debug logging
    }

    MHWRender::DrawAPI supportedDrawAPIs() const override
    {
        return kAllDevices; // OpenGL + DX
    }

    bool hasUIDrawables() const override { return true; }

    void addUIDrawables(
        const MDagPath& path,
        MHWRender::MUIDrawManager& drawManager,
        const MHWRender::MFrameContext& frameContext,
        const MUserData* data) override;


    void drawBoundingBox(MHWRender::MUIDrawManager& drawManager,
        const MBoundingBox& box,
        const MColor& color = MColor(0.0f, 1.0f, 0.0f),
        float lineWidth = 1.0f);

    MUserData* prepareForDraw(
        const MDagPath& objPath,
        const MDagPath& cameraPath,
        const MHWRender::MFrameContext& frameContext,
        MUserData* oldData) override;

};
