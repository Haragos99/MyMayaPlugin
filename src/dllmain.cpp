// dllmain.cpp : Defines the entry point for the DLL application.
#include "plugin.h"
#include <maya/MFnPlugin.h>

static MCallbackIdArray g_callbackIds;




MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Haragos", "1.0", "Any");
    return plugin.registerCommand("helloMaya", MyPluginCmd::creator);
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);

    MMessage::removeCallbacks(g_callbackIds);
    g_callbackIds.clear();

    return plugin.deregisterCommand("helloMaya");
}

