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