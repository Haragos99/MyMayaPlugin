#pragma once
#include "pch.h"


class MyPluginCmd : public MPxCommand {

private:
    MStatus createCube();
public:
    MStatus doIt(const MArgList&) override {
        MGlobal::displayInfo("A fCube the mesh created successfully.");
        return MS::kSuccess;
    }
    static void* creator() { return new MyPluginCmd; }
};
