#pragma once
#define NOMINMAX

#include <maya/MPxLocatorNode.h>
#include <maya/MDrawRegistry.h>
#include <maya/M3dView.h>

class MyLocator : public MPxLocatorNode
{
public:
    static MTypeId id;
    static void* creator() { return new MyLocator(); }
    static MStatus initialize() { return MS::kSuccess; }

    void draw(M3dView& view,
        const MDagPath& path,
        M3dView::DisplayStyle style,
        M3dView::DisplayStatus status) override
    {
        view.beginGL();
        glPushAttrib(GL_CURRENT_BIT);
        MGlobal::displayError("No son.");
        // Draw a point
        glColor3f(1.0f, 0.0f, 0.0f);  // red
        glPointSize(8.0f);
        glBegin(GL_POINTS);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glEnd();

        // Draw a line
        glColor3f(0.0f, 1.0f, 0.0f);  // green
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);
        glEnd();

        glPopAttrib();
        view.endGL();
    }
};
MTypeId MyLocator::id(0x001226C1);