// dllmain.cpp : Defines the entry point for the DLL application.
#include "plugin.h"
#include "deltamushnode.h"
#include "debugdraweroverride.h"

#include <maya/MFnPlugin.h>


MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Haragos", "1.0", "Any");

    plugin.registerNode(
        "deltaMushNode",               //  the type name Maya knows
        DeltaMushNode::id,             // the unique typeId (MTypeId)
        DeltaMushNode::creator,
        DeltaMushNode::initialize,
        MPxNode::kDeformerNode
    );
    /*
    plugin.registerNode("myLocator", MyLocator::id,
        MyLocator::creator,
        MyLocator::initialize,
        MPxNode::kLocatorNode);

    */
    return plugin.registerCommand("helloMaya", MyPluginCmd::creator);
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);

    return plugin.deregisterCommand("helloMaya");
}

