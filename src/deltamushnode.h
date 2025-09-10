#pragma once
#include "framework.h"
#include "deltamush.h"

class DeltaMushNode : public MPxDeformerNode
{
public:
    static void* creator() { return new DeltaMushNode; }
    static MStatus initialize() { return MS::kSuccess; }
    MStatus deform(MDataBlock& data,
        MItGeometry& itGeo,
        const MMatrix& localToWorldMatrix,
        unsigned int geomIndex) override;

    static MTypeId id;
    static  std::shared_ptr<DeltaMush> g_deltamushCache;
    void postConstructor()
    {
        MPxDeformerNode::setDeformationDetails(MPxDeformerNode::kDeformsColors);
    }

};
