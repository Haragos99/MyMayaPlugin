#include "plugin.h"
#include "deltamushnode.h"
#include "debugdraweroverride.h"
#include <maya/MDrawRegistry.h>
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
    const MString myClassification("drawdb/geometry/myInitials_myLocator");

        st = plugin.registerNode(
        "myLocator",               // name used in Maya (createNode myLocator)
        MyLocator::id,             // unique type ID
        MyLocator::creator,        // function pointer that creates an instance
        MyLocator::initialize,     // initialization (attributes, etc.)
        MPxNode::kLocatorNode,      // tells Maya it's a locator
        &myClassification
    );
        st.perror("registerNode"); // This will print an error if registration fails
       
      st = MDrawRegistry::registerDrawOverrideCreator(
          myClassification,
            "MyInitials_myLocatorDrawOverride",
            MyLocatorDrawOverride::Creator
        );
      st.perror("REWGregisterNode"); // This will print an error if registration fails
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

