#pragma once
#include "framework.h"
#include "deltamush.h"

class MyPluginCmd : public MPxCommand {

private:

    static std::shared_ptr<DeltaMush> deltamush;
    MStatus createCube();
    MStatus smoothMesh(MObject& meshObj, int iterations);
    static MCallbackIdArray g_callbackIds;

public:
    MStatus doIt(const MArgList&) override;
    static void* creator() { return new MyPluginCmd; }
};
