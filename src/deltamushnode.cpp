#include "deltamushnode.h"

MTypeId DeltaMushNode::id(0x00123456);

std::shared_ptr<DeltaMush> DeltaMushNode::g_deltamushCache = nullptr;


MStatus DeltaMushNode::deform(MDataBlock& data,
    MItGeometry& itGeo,
    const MMatrix& localToWorldMatrix,
    unsigned int geomIndex)
{
    MStatus status;
    MArrayDataHandle inputArray = data.inputArrayValue(input, &status);
    inputArray.jumpToElement(geomIndex);

    MDataHandle inputGeomData = inputArray.inputValue().child(inputGeom);
    MObject meshObj = inputGeomData.asMesh();  // or asMeshTransformed()
    MFnMesh fnMesh(meshObj, &status);
    MGlobal::displayInfo("NODE ");
    MPointArray pa;


    if (g_deltamushCache != nullptr)
    {
        MGlobal::displayInfo(fnMesh.name());
        //g_deltamushCache->CalculateDeformation();
        itGeo.allPositions(pa);
        g_deltamushCache->test(pa);
        
    }
    else
    {
        MGlobal::displayInfo("Bad ");
    }
    

    // Iterate over skinned geometry
    for (; !itGeo.isDone(); itGeo.next())
    {
        unsigned int vid = itGeo.index();

        // 1. Get the CURRENT vertex position (after skinning)
        MPoint skinnedPos = itGeo.position();
        MPoint avgPos(1, 1, 1);
        skinnedPos += avgPos;
       // itGeo.setPosition(avgPos);
        break;
    }
    //g_deltamushCache->move();



    return MS::kSuccess;
}