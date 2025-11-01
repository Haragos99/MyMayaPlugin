#include "plugin.h"
#include "deltamushnode.h"
#include <maya/MFnLambertShader.h>
#include <maya/MFnSet.h>
#include "debugdraweroverride.h"
MCallbackIdArray MyPluginCmd::g_callbackIds;
std::shared_ptr<DeltaMush> MyPluginCmd::deltamush;


MStatus MyPluginCmd::smoothMesh(MObject& meshObj, int iterations)
{
    MStatus status;
    double smoothingFactor = 0.5;
    MFnMesh meshFn(meshObj, &status);
    if (!status) 
    {
        return status;
    }
    unsigned int vertexCount = meshFn.numVertices();
    MPointArray currentPoints;
    meshFn.getPoints(currentPoints, MSpace::kObject);

    for (int i = 0; i < iterations; ++i)
    {
        MPointArray updatedPoints = currentPoints;// create copy of the points

        MItMeshVertex itVert(meshObj, &status);
        if (!status) 
        { 
            return status; 
        }

        while (!itVert.isDone())
        {
            int idx = itVert.index();
            MIntArray connectedVerticesIndices; 
            itVert.getConnectedVertices(connectedVerticesIndices); // get indeces of the verteces
            bool hasNeighbors = connectedVerticesIndices.length() > 0;
            if (hasNeighbors)
            {
                MPoint avgPos(0.0, 0.0, 0.0);
                for (int j = 0; j < connectedVerticesIndices.length(); ++j)
                {
                    avgPos += currentPoints[connectedVerticesIndices[j]];
                }
                avgPos = avgPos /connectedVerticesIndices.length();

                MVector diff = avgPos - currentPoints[idx];
                updatedPoints[idx] += diff * smoothingFactor;
            }
            
            itVert.next();
        }
        currentPoints = updatedPoints;  
    }
    // Apply new positions
    status = meshFn.setPoints(currentPoints, MSpace::kObject);
    return status;
}





MStatus MyPluginCmd::doIt(const MArgList&)
{
    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);

    if (selection.length() == 0)
    {
        MGlobal::displayError("No object selected.");
        return MS::kFailure;
    }

    MStatus status;
    MString nodeType("deltaMushNode");
    MFnDependencyNode fnDep;
    MObject node = fnDep.create(nodeType, &status);
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
    MString cmd1 = "polyTriangulate " + dagPath.fullPathName();
    //MGlobal::executeCommand(cmd1);
    deltamush = std::make_shared<DeltaMush>(dagPath);
    deltamush->CalculateDelta();
    deltamush->CalculateDeformation();



    // TODO: Refactor this for non static solution  
    DeltaMushNode::g_deltamushCache = deltamush;
    MyLocatorDrawOverride::deltamushCache = deltamush;
    MString cmd;
    cmd.format("deformer -type \"^1s\" ^2s;", nodeType, dagPath.fullPathName());
    MGlobal::executeCommand(cmd);

    MGlobal::displayInfo("Mesh modified successfully.");
    return MStatus::kSuccess;
}


MStatus MyPluginCmd::createCube()
{
    // When you are using pre-compiled headers, this source file is necessary for compilation to succeed.
    MStatus status;

    // Define the 8 vertices of the cube
    MPointArray vertices;
    vertices.append(MPoint(-1.0, -1.0, -1.0)); // Vertex 0
    vertices.append(MPoint(1.0, -1.0, -1.0)); // Vertex 1
    vertices.append(MPoint(1.0, -1.0, 1.0)); // Vertex 2
    vertices.append(MPoint(-1.0, -1.0, 1.0)); // Vertex 3
    vertices.append(MPoint(-1.0, 1.0, -1.0)); // Vertex 4
    vertices.append(MPoint(1.0, 1.0, -1.0)); // Vertex 5
    vertices.append(MPoint(1.0, 1.0, 1.0)); // Vertex 6
    vertices.append(MPoint(-1.0, 1.0, 1.0)); // Vertex 7

    // Define the number of vertices per face (6 faces, each with 4 vertices)
    MIntArray polygonCounts;
    for (int i = 0; i < 6; ++i) {
        polygonCounts.append(4);
    }

    // Define the vertex indices for each face
    MIntArray polygonConnects;
    // Bottom face
    polygonConnects.append(0); polygonConnects.append(1);
    polygonConnects.append(2); polygonConnects.append(3);
    // Top face
    polygonConnects.append(4); polygonConnects.append(5);
    polygonConnects.append(6); polygonConnects.append(7);
    // Front face
    polygonConnects.append(3); polygonConnects.append(2);
    polygonConnects.append(6); polygonConnects.append(7);
    // Back face
    polygonConnects.append(0); polygonConnects.append(1);
    polygonConnects.append(5); polygonConnects.append(4);
    // Left face
    polygonConnects.append(0); polygonConnects.append(3);
    polygonConnects.append(7); polygonConnects.append(4);
    // Right face
    polygonConnects.append(1); polygonConnects.append(2);
    polygonConnects.append(6); polygonConnects.append(5);

    // Create the mesh
    MFnMesh meshFn;
    MObject mesh = meshFn.create(
        vertices.length(),            // Number of vertices
        polygonCounts.length(),       // Number of polygons
        vertices,                     // Vertex positions
        polygonCounts,                // Number of vertices per polygon
        polygonConnects,              // Vertex indices
        MObject::kNullObj,            // Parent or owner (optional)
        &status
    );

    if (status != MStatus::kSuccess) {
        MGlobal::displayError("Failed to create the cube mesh.");
        return status;
    }

    MGlobal::displayInfo("A Cube the mesh created successfully.");

}




