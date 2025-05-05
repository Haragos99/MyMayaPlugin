#pragma once
#include "pch.h"


class MyPluginCmd : public MPxCommand {

private:
    MStatus createCube();
    MStatus smoothMesh(MObject& meshObj, int iterations);

public:
    MStatus doIt(const MArgList&) override;
    static void* creator() { return new MyPluginCmd; }
};
