#pragma once
#include "framework.h"
#include "deltamush.h"

class DeltaMushNode : public MPxDeformerNode
{
public:
    static void* creator() { return new DeltaMushNode; }
    static MStatus initialize();
    void postConstructor();
    MStatus deform(MDataBlock& data,
        MItGeometry& itGeo,
        const MMatrix& localToWorldMatrix,
        unsigned int geomIndex) override;
    static MObject aEnableFeature; // Boolean attribute handle
    static MObject aEnableDeltamush; // Boolean attribute handle
    static MObject aSmoothing;       //  float slider
	static MObject aCounter; 	 //  int 
    static MObject aStrength; 	 //  float slider
    static MObject aEnableDebug;
    static MObject deformerMsgAttr;
    static MTypeId id;

    MStatus setDependentsDirty(const MPlug& plugBeingDirtied, MPlugArray& affectedPlugs) override;
    std::shared_ptr<DeltaMush> m_deltamush;
};
