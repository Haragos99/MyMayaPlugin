#pragma once
#include "framework.h"
#include "deltamush.h"

class DeltaMushNode : public MPxDeformerNode
{
public:
    static void* creator() { return new DeltaMushNode; }
    static MStatus initialize();
    MStatus deform(MDataBlock& data,
        MItGeometry& itGeo,
        const MMatrix& localToWorldMatrix,
        unsigned int geomIndex) override;
    static MObject aEnableFeature; // Boolean attribute handle
    static MObject aEnableDeltamush; // Boolean attribute handle
    static MObject aSmoothing;       //  float slider
	static MObject aCounter; 	 //  int 
    static MObject aEnableDebug;
    static MTypeId id;
    // --- Overridden methods ---
    MStatus setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs) override;
    static  std::shared_ptr<DeltaMush> g_deltamushCache;
    void postConstructor()
    {
        MPxDeformerNode::setDeformationDetails(MPxDeformerNode::kDeformsColors);
    }

};
