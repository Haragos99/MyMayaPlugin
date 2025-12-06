#pragma once
#include "framework.h"
#include "deltamush.h"

class MyPluginCmd : public MPxCommand {

private:
    static unsigned int nextId;
	MStatus createDeltaMush();
    MStatus createDrawLocator();
    static MCallbackIdArray g_callbackIds;
    MeshHandler smoothMesh(MeshHandler mesh, int iterations);
    MString deformerNodeType;
    MString locatorNodeType;
    unsigned int nodeId;
public:
    MyPluginCmd();
    MStatus doIt(const MArgList&) override;
    static void* creator() { return new MyPluginCmd; }
};
