#include "deltamushnode.h"

MTypeId DeltaMushNode::id(0x00123456);

std::shared_ptr<DeltaMush> DeltaMushNode::g_deltamushCache = nullptr;


MStatus DeltaMushNode::deform(MDataBlock& data,
    MItGeometry& itGeo,
    const MMatrix& localToWorldMatrix,
    unsigned int geomIndex)
{
    MStatus status;
    MPointArray points;
    if (g_deltamushCache != nullptr)
    {
        itGeo.allPositions(points);
        g_deltamushCache->test(points); 
    }
    else
    {
        MGlobal::displayError("Delta Mush is null"); 
    }
    
    return MS::kSuccess;
}