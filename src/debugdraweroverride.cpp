#include "debugdraweroverride.h"

MTypeId MyLocator::id(0x001226C1);



std::shared_ptr<DeltaMush> MyLocatorDrawOverride::deltamushCache = nullptr;

void MyLocatorDrawOverride::addUIDrawables(
    const MDagPath& path,
    MHWRender::MUIDrawManager& drawManager,
    const MHWRender::MFrameContext& frameContext,
    const MUserData* data) 
{
    drawManager.beginDrawable();
    MGlobal::displayInfo("SSSSSIt worked");
    // Draw red point
    drawManager.setColor(MColor(1.0f, 0.0f, 0.0f));
    drawManager.point(MPoint(0.0, 0.0, 0.0));


    


    // Draw green line
    drawManager.setColor(MColor(0.0f, 1.0f, 0.0f));
    drawManager.line(MPoint(0.0, 0.0, 0.0), MPoint(1.0, 1.0, 0.0));

    drawManager.endDrawable();
}