// dllmain.cpp : Defines the entry point for the DLL application.
#include "plugin.h"
#include <maya/MFnPlugin.h>

static MCallbackIdArray g_callbackIds;




void onTransformChanged(MObject& node, MDagMessage::MatrixModifiedFlags& modificationType, void* clientData)
{
    MFnDependencyNode fn(node);
    MGlobal::displayInfo("Transform changed on node: " + fn.name());
}




void onAttrChanged(MNodeMessage::AttributeMessage msg,
    MPlug& plug,
    MPlug& otherPlug,
    void* clientData)
{
    if (msg & MNodeMessage::kAttributeSet)
    {
        MString attr = plug.partialName();
        if (attr == "tx" || attr == "ty" || attr == "tz")
        {
            MGlobal::displayInfo("Translation changed: " + attr);
        }
    }
}



MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Haragos", "1.0", "Any");

    MDagPath node;
    MSelectionList sel;
    MGlobal::getActiveSelectionList(sel);
    sel.getDagPath(0, node);  // Get first selected item
    sel.add("pCube1");  // pCube1 is the transform, pCubeShape1 is the shape
    MObject transformNode;
    sel.getDependNode(0, transformNode);
    try
    {
        if (!transformNode.isNull())
        {
            MStatus status;
            //MCallbackId id = MDagMessage::addWorldMatrixModifiedCallback(node, onTransformChanged, nullptr, &status);
            MCallbackId id = MNodeMessage::addAttributeChangedCallback(
                transformNode,
                onAttrChanged,
                nullptr);
            if (status != MS::kSuccess)
            {
                MGlobal::displayError("Failed to add callback");
            } 
            else
            {
                g_callbackIds.append(id);
            }              
        }
        else 
        {
            MGlobal::displayError("Node is invalid!");
        }

    }
    catch (const std::exception& e)
    {
        MGlobal::displayError(e.what());
    }
    
    
    return plugin.registerCommand("helloMaya", MyPluginCmd::creator);
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);

    MMessage::removeCallbacks(g_callbackIds);
    g_callbackIds.clear();

    return plugin.deregisterCommand("helloMaya");
}

