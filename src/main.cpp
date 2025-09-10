#include "plugin.h"
#include "deltamushnode.h"
#include "debugdraweroverride.h"

#include <maya/MFnPlugin.h>


MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Haragos", "1.0", "Any");
    MStatus st;
    st=plugin.registerNode(
        "deltaMushNode",               //  the type name Maya knows
        DeltaMushNode::id,             // the unique typeId (MTypeId)
        DeltaMushNode::creator,
        DeltaMushNode::initialize,
        MPxNode::kDeformerNode
    );

    if (st == MS::kSuccess)
    {
        MGlobal::displayInfo("It worked");
    }

    return plugin.registerCommand("helloMaya", MyPluginCmd::creator);
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);

    plugin.deregisterNode(DeltaMushNode::id);
    return plugin.deregisterCommand("helloMaya");
}

