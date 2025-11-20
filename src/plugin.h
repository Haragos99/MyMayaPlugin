#pragma once
#include "framework.h"
#include "deltamush.h"


class MyPluginCmd : public MPxCommand {

private:

    static std::shared_ptr<DeltaMush> deltamush;
    MStatus createCube();
    MFnMesh& smoothMesh(MObject& meshObj, int iterations);
    MStatus collisonTest(MDagPath& dagPath0, MDagPath& dagPath1);
    MStatus collisonTest2(MDagPath& dagPath0, MDagPath& dagPath1);
	MStatus runDeltaMush();
    static MCallbackIdArray g_callbackIds;
    bool isPointInPlane(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c);
    bool isPointInTriangle(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c);
    std::set<int> collidedVertices;
    double mixed(MVector& a, MVector& b, MVector& c);
    MStatus THecollisonTest3(MDagPath& dagPath0);
    MStatus collison(MDagPath& dagPath0, MDagPath& dagPath1);
    MeshHandler smoothMesh(MeshHandler mesh, int iterations);

public:
    MStatus doIt(const MArgList&) override;
    static void* creator() { return new MyPluginCmd; }
};
