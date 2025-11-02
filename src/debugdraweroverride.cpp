#include "debugdraweroverride.h"

MTypeId MyLocator::id(0x001226C1);

std::shared_ptr<DeltaMush> MyLocatorDrawOverride::deltamushCache = nullptr;

// TODO: Refactor for other solution to use MUserData
void MyLocatorDrawOverride::addUIDrawables(
    const MDagPath& path,
    MHWRender::MUIDrawManager& drawManager,
    const MHWRender::MFrameContext& frameContext,
    const MUserData* data) 
{
    drawManager.beginDrawable();
    MGlobal::displayInfo("SSSSSIt worked");

    auto mesh = deltamushCache->getMeshHandler();
    auto smoothmesh = deltamushCache->getSmoothMeshHandler();
    auto deltas = deltamushCache->getDeltas();

    auto points = mesh.getVertices();

    // Draw red point
    drawManager.setColor(MColor(1.0f, 0.0f, 0.0f));
    drawManager.point(MPoint(0.0, 0.0, 0.0));
    float factor = deltamushCache->getDeltaMushFactor();

    if (factor < 1)
    {
        for (int i = 0; i < points.length(); ++i)
        {
            drawManager.setColor(MColor(1.0f, 0.0f, 0.0f));
            MPoint delta = deltas[i];
            MPoint start = mesh.getMatrixC(i) * delta;
            delta[0] *= factor;
            delta[1] *= factor;
            delta[2] *= factor;
            MPoint end = mesh.getMatrixC(i) * delta;
            drawManager.line(start, end);
        }
    }

    // Draw green line
    drawManager.setColor(MColor(0.0f, 1.0f, 0.0f));
    drawManager.line(MPoint(0.0, 0.0, 0.0), MPoint(1.0, 1.0, 0.0));

    drawManager.endDrawable();
}




// Draws a bounding box (outline) for the given box in world space
void MyLocatorDrawOverride::drawBoundingBox(MHWRender::MUIDrawManager& drawManager,
    const MBoundingBox& box,
    const MColor& color = MColor(0.0f, 1.0f, 0.0f),
    float lineWidth = 1.0f)
{

    MPoint minP = box.min();
    MPoint maxP = box.max();

    MPoint corners[8] = {
        MPoint(minP.x, minP.y, minP.z),
        MPoint(maxP.x, minP.y, minP.z),
        MPoint(maxP.x, maxP.y, minP.z),
        MPoint(minP.x, maxP.y, minP.z),
        MPoint(minP.x, minP.y, maxP.z),
        MPoint(maxP.x, minP.y, maxP.z),
        MPoint(maxP.x, maxP.y, maxP.z),
        MPoint(minP.x, maxP.y, maxP.z)
    };

    drawManager.setColor(color);
    drawManager.setLineWidth(lineWidth);

    // bottom face
    drawManager.line(corners[0], corners[1]);
    drawManager.line(corners[1], corners[2]);
    drawManager.line(corners[2], corners[3]);
    drawManager.line(corners[3], corners[0]);

    // top face
    drawManager.line(corners[4], corners[5]);
    drawManager.line(corners[5], corners[6]);
    drawManager.line(corners[6], corners[7]);
    drawManager.line(corners[7], corners[4]);

    // vertical edges
    drawManager.line(corners[0], corners[4]);
    drawManager.line(corners[1], corners[5]);
    drawManager.line(corners[2], corners[6]);
    drawManager.line(corners[3], corners[7]);

}
