#include "deltamushnode.h"

MTypeId DeltaMushNode::id(0x00123456);

std::shared_ptr<DeltaMush> DeltaMushNode::g_deltamushCache = nullptr;
MObject DeltaMushNode::aEnableFeature;
MObject DeltaMushNode::aEnableDeltamush;
MObject DeltaMushNode::aSmoothing;
MObject DeltaMushNode::aCounter;
MObject DeltaMushNode::aEnableDebug;

// --- Attribute initialization ---
MStatus DeltaMushNode::initialize()
{
    MFnNumericAttribute nAttr;

    //  Create a boolean attribute
    aEnableFeature = nAttr.create("ImproveDeltaMush", "eni", MFnNumericData::kBoolean, false);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setDefault(false);

    addAttribute(aEnableFeature);

    // Ensure Maya knows the attribute affects the output geometry
    attributeAffects(aEnableFeature, outputGeom);




    //  Create a boolean attribute
    aEnableDeltamush = nAttr.create("DeltaMush", "dm", MFnNumericData::kBoolean, true);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setDefault(true);

    addAttribute(aEnableDeltamush);

    // Ensure Maya knows the attribute affects the output geometry
    attributeAffects(aEnableDeltamush, outputGeom);





    //  Create a boolean attribute
    aEnableDebug = nAttr.create("Debugg IMD", "debugg", MFnNumericData::kBoolean, false);
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    nAttr.setDefault(false);

    addAttribute(aEnableDebug);

    // Ensure Maya knows the attribute affects the output geometry
    attributeAffects(aEnableDebug, outputGeom);



    // Smoothing (float slider)
    aSmoothing = nAttr.create("smoothing", "sm", MFnNumericData::kFloat, 100.0f);
    nAttr.setMin(0.0f);     // slider min
    nAttr.setMax(100.0f);    // slider max
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    addAttribute(aSmoothing);

    attributeAffects(aSmoothing, outputGeom);


	//create ancounter for debugging IMD 
    aCounter = nAttr.create("counter", "cnt", MFnNumericData::kInt, 0);
    nAttr.setMin(0); 
    nAttr.setMax(100);
    nAttr.setKeyable(true);         
    nAttr.setStorable(true);        
    nAttr.setReadable(true);         
    nAttr.setWritable(true);         
    addAttribute(aCounter);

    attributeAffects(aCounter, outputGeom);



    return MS::kSuccess;
}



MStatus DeltaMushNode::setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs)
{
    if (plugBeingDirtied == aSmoothing)
    {
        // Read the new value safely from the plug
        float smoothVal = 0.0f;
        plugBeingDirtied.getValue(smoothVal); // works because it's a numeric attribute

        // Call your custom method
        g_deltamushCache->setDeltaMushFactor(smoothVal);
		g_deltamushCache->CalculateDeformation();
		MGlobal::displayInfo(MString("Smoothing changed to: ") + smoothVal);
    }

    if(plugBeingDirtied == aCounter)
    {
        int counter = 0;
        plugBeingDirtied.getValue(counter); // works because it's a numeric attribute
        // Call your custom method
		g_deltamushCache->debugCCD(counter, g_deltamushCache->getPoints());
        MGlobal::displayInfo(MString("Counter changed to: ") + counter);
	}
	return MPxDeformerNode::setDependentsDirty(plugBeingDirtied, affectedPlugs);
}


MStatus DeltaMushNode::deform(MDataBlock& data,
    MItGeometry& itGeo,
    const MMatrix& localToWorldMatrix,
    unsigned int geomIndex)
{
    MStatus status;
    bool enableFeature = data.inputValue(aEnableFeature, &status).asBool();

	bool enableDeltamush = data.inputValue(aEnableDeltamush, &status).asBool();

    float smoothing = data.inputValue(aSmoothing, &status).asFloat();
	bool enableDebug = data.inputValue(aEnableDebug, &status).asBool();
	int counter = data.inputValue(aCounter, &status).asInt();
    MPointArray points;
    if(enableDeltamush)
    {
        if (g_deltamushCache != nullptr)
        {

            g_deltamushCache->setDeltaMushFactor(smoothing);
            itGeo.allPositions(points);
            if (enableFeature)
            {
                if(enableDebug)
                {
                    //g_deltamushCache->debugCCD(counter, points);
                }
                else
                {
                    g_deltamushCache->improvedDM(points);
                }
               
            }
            else
            {
                g_deltamushCache->test(points);
            }

        }
        else
        {
            MGlobal::displayError("Delta Mush is null");
        }
	}
   
    return MS::kSuccess;
}