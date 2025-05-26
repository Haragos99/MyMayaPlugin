#pragma once
#include "framework.h"
#include "deltamush.h"

class MyPluginCmd : public MPxCommand {

private:

    std::unique_ptr<DeltaMush> deltamush;
    MStatus createCube();
    MStatus smoothMesh(MObject& meshObj, int iterations);

public:
    MStatus doIt(const MArgList&) override;
    static void* creator() { return new MyPluginCmd; }
    // Static function to call from callbacks
    static void onTransformChanged(const MString& nodeName);
};
