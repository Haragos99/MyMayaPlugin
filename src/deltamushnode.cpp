#include "deltamushnode.h"

MTypeId DeltaMushNode::id(0x00123456);

MObject DeltaMushNode::aEnableFeature;
MObject DeltaMushNode::aEnableDeltamush;
MObject DeltaMushNode::aSmoothing;
MObject DeltaMushNode::aCounter;
MObject DeltaMushNode::aEnableDebug;
MObject DeltaMushNode::aStrength;
MObject DeltaMushNode::deformerMsgAttr;

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

    // Strength  (float slider)
    aStrength = nAttr.create("strength", "st", MFnNumericData::kFloat, 0.5f);
    nAttr.setMin(0.0f);     // slider min
    nAttr.setMax(1.0f);    // slider max
    nAttr.setKeyable(true);
    nAttr.setStorable(true);
    nAttr.setReadable(true);
    nAttr.setWritable(true);
    addAttribute(aStrength);

    attributeAffects(aStrength, outputGeom);







    MFnTypedAttribute tAttr;
    deformerMsgAttr = tAttr.create("message", "msg", MFnData::kString);
    tAttr.setStorable(false);
    tAttr.setWritable(false);
    tAttr.setReadable(true);

    addAttribute(deformerMsgAttr);


    return MS::kSuccess;
}

void DeltaMushNode::postConstructor()
{
    MPxDeformerNode::setDeformationDetails(MPxDeformerNode::kDeformsColors);
    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);

    if (selection.length() == 0)
    {
        MGlobal::displayError("No object selected.");
    }

    MDagPath dagPath;
    selection.getDagPath(0, dagPath);  // Get first selected item
    MGlobal::displayInfo("Mesh Path for DelatMush: " + dagPath.fullPathName());
	// initialize DeltaMush 
    m_deltamush = std::make_shared<DeltaMush>(dagPath);
	m_deltamush->CalculateDelta();
}


MStatus DeltaMushNode::setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs)
{
    if (plugBeingDirtied == aSmoothing)
    {
        // Read the new value safely from the plug
        float smoothVal = 0.0f;
        plugBeingDirtied.getValue(smoothVal); // works because it's a numeric attribute

        // Call your custom method
        m_deltamush->setDeltaMushFactor(smoothVal);
		m_deltamush->CalculateDeformation();
		MGlobal::displayInfo(MString("Smoothing changed to: ") + smoothVal);
        MDagPath dagPath;
        MDagPath::getAPathTo(thisMObject(), dagPath);
        MHWRender::MRenderer::setGeometryDrawDirty(dagPath.node());
    }

    if(plugBeingDirtied == aCounter)
    {
        int counter = 0;
        plugBeingDirtied.getValue(counter); // works because it's a numeric attribute
        // Call your custom method
		//g_deltamushCache->debugCCD(counter, g_deltamushCache->getPoints());
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

	float strength = data.inputValue(aStrength, &status).asFloat();
    
    MPointArray points;
    if(enableDeltamush)
    {
        if (m_deltamush != nullptr)
        {
            m_deltamush->setDeltaMushFactor(smoothing);
            itGeo.allPositions(points);
            if (enableFeature)
            {
				m_deltamush->setStrength(strength);
                if(enableDebug)
                {
                    m_deltamush->debugCCD(counter, points);
                }
                else
                {
                    m_deltamush->improvedDM(points);
                }
            }
            else
            {
                m_deltamush->test(points);
            }
        }
        else
        {
            MGlobal::displayError("Delta Mush is null");
        }
	}
   
    return MS::kSuccess;
}