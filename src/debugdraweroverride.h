#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>
#include <maya/MGlobal.h>
#include <maya/MPxLocatorNode.h>
#include "deltamush.h"
using namespace MHWRender;



class MyLocator : public MPxLocatorNode
{
public:
    static MTypeId id;

    static void* creator() { return new MyLocator(); }
    static MStatus initialize() { return MS::kSuccess; }

    // Only used in Legacy Viewport (optional)
    void draw(M3dView& view,
        const MDagPath& path,
        M3dView::DisplayStyle style,
        M3dView::DisplayStatus status) override
    {
        view.beginGL();
        glPushAttrib(GL_CURRENT_BIT);

        // Red point
        glColor3f(1.0f, 0.0f, 0.0f);
        glPointSize(8.0f);
        glBegin(GL_POINTS);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glEnd();
        MGlobal::displayInfo("A cx It worked");
        // Green line
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glEnd();

        glPopAttrib();
        view.endGL();
    }
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


    static  std::shared_ptr<DeltaMush> deltamushCache;

	static std::vector<MPoint> CDpoints;
    static std::vector<MPointArray> CDfaces;

    void drawBoundingBox(MHWRender::MUIDrawManager& drawManager,
        const MBoundingBox& box,
        const MColor& color = MColor(0.0f, 1.0f, 0.0f),
        float lineWidth = 1.0f);


    MUserData* prepareForDraw(
        const MDagPath& objPath,
        const MDagPath& cameraPath,
        const MHWRender::MFrameContext& frameContext,
        MUserData* oldData) override
    {
        // If you want to cache per-frame draw data, do it here.
        // For now, just reuse oldData if it exists.
        return oldData;
    }

};
