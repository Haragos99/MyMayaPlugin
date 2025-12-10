#include "plugin.h"
#include "deltamushnode.h"
#include <maya/MFnLambertShader.h>
#include <maya/MFnSet.h>
#include "debugdraweroverride.h"
#include "intersectionfilter.h"
MCallbackIdArray MyPluginCmd::g_callbackIds;
unsigned int MyPluginCmd::nextId = 0;

MyPluginCmd::MyPluginCmd()
{
    deformerNodeType = "deltaMushNode";
    locatorNodeType = "myLocator";
	nodeId = 0;
}

MStatus MyPluginCmd::doIt(const MArgList&)
{
	nodeId = nextId++; // assign and increment unique node ID
	MStatus status;
    status = createDeltaMush();
    status = createDrawLocator();
    return status;
}

// Create Delta Mush Deformer 
MStatus MyPluginCmd::createDeltaMush()
{
    MStatus status;
    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);

    if (selection.length() == 0)
    {
        MGlobal::displayError("No object selected.");
        return MS::kFailure;
    }

    MFnDependencyNode fnDep;
    MDagPath dagPath;
    selection.getDagPath(0, dagPath);  // Get first selected item
    MGlobal::displayInfo("Add mesh for: " + dagPath.fullPathName());
    MItSelectionList iter(selection, MFn::kMesh);
    MObject meshObj;
    iter.getDependNode(meshObj);

    if (meshObj.isNull())
    {
        MGlobal::displayError("No mesh found in selection.");
        return MS::kFailure;
    }
    MString deltaMushNodeName = deformerNodeType + MString("_") + MString(std::to_string(nodeId).c_str());
    MString cmd;
    cmd.format("deformer -type \"^1s\" -name \"^2s\" ^3s;", deformerNodeType, deltaMushNodeName, dagPath.fullPathName());
    MGlobal::executeCommand(cmd);

    MGlobal::displayInfo("Delta Mush created");
	return MS::kSuccess;
}

// Create Locator for visualizing Deformer
MStatus MyPluginCmd::createDrawLocator()
{
    MString deltaMushNodeName = deformerNodeType + MString("_") + MString(std::to_string(nodeId).c_str());
    MString locatorName = "myLocator_" + MString(std::to_string(nodeId).c_str());

    MString cmd;
    cmd.format("createNode myLocator -name \"^1s\";", locatorName);
    MGlobal::executeCommand(cmd);

	// create connection between deltaMushNode and locator
    MStatus status = MGlobal::executeCommand(MString("connectAttr ")
        + deltaMushNodeName + ".message "
        + locatorName + ".deformerMessage");

    if (!status) 
    {
        MGlobal::displayError("Failed to connect deformer to locator!");
		return MS::kFailure;
    }
    MGlobal::displayInfo("Locator node created");
	return MStatus::kSuccess;
}