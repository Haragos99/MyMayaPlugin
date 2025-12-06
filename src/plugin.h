#pragma once
#include "framework.h"
#include "deltamush.h"

class MyPluginCmd : public MPxCommand {

private:
    static unsigned int nextId;
    MStatus createCube();
    MFnMesh& smoothMesh(MObject& meshObj, int iterations);
    MStatus collisonTest(MDagPath& dagPath0, MDagPath& dagPath1);
    MStatus collisonTest2(MDagPath& dagPath0, MDagPath& dagPath1);
	MStatus createDeltaMush();
    MStatus createDrawLocator();
    static MCallbackIdArray g_callbackIds;
    std::set<int> collidedVertices;
    MStatus collison(MDagPath& dagPath0, MDagPath& dagPath1);
    MeshHandler smoothMesh(MeshHandler mesh, int iterations);
    MString deformerNodeType;
    MString locatorNodeType;
    unsigned int nodeId;
public:
    MyPluginCmd();
    MStatus doIt(const MArgList&) override;
    static void* creator() { return new MyPluginCmd; }
};
