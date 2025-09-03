#pragma once
#include "framework.h"
#include "deltamush.h"

class MyPluginCmd : public MPxCommand {

private:

    static std::shared_ptr<DeltaMush> deltamush;
    MStatus createCube();
    MStatus smoothMesh(MObject& meshObj, int iterations);
    static MCallbackIdArray g_callbackIds;
    // Helper to draw a line
    
public:
    static void onAttrChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* clientData);
    MStatus doIt(const MArgList&) override;
    static void* creator() { return new MyPluginCmd; }
    // Static function to call from callbacks
    static void onTransformChanged(const MString& nodeName);
};
