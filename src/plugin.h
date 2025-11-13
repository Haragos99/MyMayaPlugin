#pragma once
#include "framework.h"
#include "deltamush.h"


class MyPluginCmd : public MPxCommand {

private:

    static std::shared_ptr<DeltaMush> deltamush;
    MStatus createCube();
    MFnMesh& smoothMesh(MObject& meshObj, int iterations);
    MStatus collisonTest();
	MStatus runDeltaMush();
    static MCallbackIdArray g_callbackIds;
    bool isPointInPlane(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c);
    bool isPointInTriangle(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c);
    double mixed(MVector& a, MVector& b, MVector& c);

public:
    MStatus doIt(const MArgList&) override;
    static void* creator() { return new MyPluginCmd; }
};
