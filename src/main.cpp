#include "plugin.h"
#include "deltamushnode.h"
#include "debugdraweroverride.h"
#include <maya/MDrawRegistry.h>
#include <maya/MFnPlugin.h>



MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Haragos", "1.0", "Any");
    MStatus status;
    status = plugin.registerNode(
        "deltaMushNode",               //  the type name Maya knows
        DeltaMushNode::id,             // the unique typeId (MTypeId)
        DeltaMushNode::creator,
        DeltaMushNode::initialize,
        MPxNode::kDeformerNode
    );

    const MString myClassification("drawdb/geometry/myInitials_myLocator");
    status = plugin.registerNode(
        "myLocator",               // name used in Maya (createNode myLocator)
        MyLocator::id,             // unique type ID
        MyLocator::creator,        // function pointer that creates an instance
        MyLocator::initialize,     // initialization (attributes, etc.)
        MPxNode::kLocatorNode,      // tells Maya it's a locator
        &myClassification
    );
    status.perror("registerNode"); // This will print an error if registration fails
       
    status = MDrawRegistry::registerDrawOverrideCreator(
          myClassification,
          "MyInitials_myLocatorDrawOverride",
          MyLocatorDrawOverride::Creator
    );
    status.perror("REWGregisterNode"); // This will print an error if registration fails

    if (status == MS::kSuccess)
    {
        MGlobal::displayInfo("All Nodes regiestered");
    }

    return plugin.registerCommand("helloMaya", MyPluginCmd::creator);
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);

    plugin.deregisterNode(DeltaMushNode::id);
	plugin.deregisterNode(MyLocator::id);
    MDrawRegistry::deregisterDrawOverrideCreator(
        "drawdb/geometry/myInitials_myLocator",
        "MyInitials_myLocatorDrawOverride"
	);
    return plugin.deregisterCommand("helloMaya");
}

