#include "plugin.h"
#include "deltamushnode.h"
#include <maya/MFnLambertShader.h>
#include <maya/MFnSet.h>
#include "debugdraweroverride.h"
#include "intersectionfilter.h"
MCallbackIdArray MyPluginCmd::g_callbackIds;
unsigned int MyPluginCmd::nextId = 0;

MyPluginCmd ::MyPluginCmd()
{
    deformerNodeType = "deltaMushNode";
    locatorNodeType = "myLocator";
}

MFnMesh& MyPluginCmd::smoothMesh(MObject& meshObj, int iterations)
{
    MStatus status;
    double smoothingFactor = 0.25;
    MFnMesh meshFn(meshObj, &status);
    if (!status) 
    {
       // return status;
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
            //return status; 
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
    return meshFn;
}



MStatus MyPluginCmd::doIt(const MArgList&)
{
	nodeId = nextId++; // assign and increment unique node ID
	MStatus status;
    status = createDeltaMush();
    status = createDrawLocator();
    return status;
}

MStatus MyPluginCmd::collison(MDagPath& dagPath0, MDagPath& dagPath1)
{
    MStatus status;
    MeshHandler mesh = MeshHandler(dagPath0);
    MeshHandler smoothmesh = MeshHandler(dagPath1);

    double smoothingFactor = 0.5;

	MPointArray currentPoints = smoothmesh.getVertices();

    for (int i = 0; i < 20; ++i)
    {
        MPointArray updatedPoints = currentPoints;// create copy of the points

        MItMeshVertex itVert(dagPath1, MObject::kNullObj, &status);
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
                avgPos = avgPos / connectedVerticesIndices.length();

                MVector diff = avgPos - currentPoints[idx];
                updatedPoints[idx] += diff * smoothingFactor;
            }

            itVert.next();
        }
        currentPoints = updatedPoints;
    }
    // Apply new positions
    smoothmesh.setVertices(currentPoints);
    MGlobal::displayInfo("Mesh modified successfully.");


    auto& points =mesh.getVertices();
    IntersectionFilter intersector = IntersectionFilter(smoothmesh);
    auto collisionPoints = intersector.filterFirstIntersections(mesh.getVertices(), mesh);
	std::vector<MPoint> collisonPointsVec;
    for (int i : collisionPoints)
    {
		collisonPointsVec.push_back(points[i]);
    }

    MGlobal::executeCommand("createNode myLocator");
    MGlobal::displayInfo("Mesh modified successfully.");

	return MStatus::kSuccess;
}

MeshHandler MyPluginCmd::smoothMesh(MeshHandler mesh, int iterations)
{
    MStatus status;
    double smoothingFactor = 0.5;
    MPointArray currentPoints = mesh.getVertices();
    for (int i = 0; i < iterations; ++i)
    {
        MPointArray updatedPoints = currentPoints;// create copy of the points
        auto itVert = mesh.getVertexIterator(&status);
        while (!itVert->isDone())
        {
            int idx = itVert->index();
            MIntArray connectedVerticesIndices;
            itVert->getConnectedVertices(connectedVerticesIndices); // get indeces of the verteces
            bool hasNeighbors = connectedVerticesIndices.length() > 0;
            if (hasNeighbors)
            {
                MPoint avgPos(0.0, 0.0, 0.0);
                for (int j = 0; j < connectedVerticesIndices.length(); ++j)
                {
                    avgPos += currentPoints[connectedVerticesIndices[j]];
                }
                avgPos = avgPos / connectedVerticesIndices.length();

                MVector diff = avgPos - currentPoints[idx];
                updatedPoints[idx] += diff * smoothingFactor;
            }
            itVert->next();
        }
        currentPoints = updatedPoints;
    }
    mesh.setVertices(currentPoints);
    MGlobal::displayInfo("sss");
    return mesh;
}


// Create Delta Mush Deformer 
MStatus MyPluginCmd::createDeltaMush()
{
    MStatus status;
    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);

    if (selection.length() == 0)
    {
        MGlobal::displayError("No object selected.");
        return MS::kFailure;
    }

    MFnDependencyNode fnDep;
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
    MString deltaMushNodeName = deformerNodeType + MString("_") + MString(std::to_string(nodeId).c_str());
    MString cmd;
    cmd.format("deformer -type \"^1s\" -name \"^2s\" ^3s;", deformerNodeType, deltaMushNodeName, dagPath.fullPathName());
    MGlobal::executeCommand(cmd);

    MGlobal::displayInfo("Delta Mush created");
	return MS::kSuccess;
}

// Create Locator for visualizing Deformer
MStatus MyPluginCmd::createDrawLocator()
{
    MString deltaMushNodeName = deformerNodeType + MString("_") + MString(std::to_string(nodeId).c_str());
    MString locatorName = "myLocator_" + MString(std::to_string(nodeId).c_str());

    MString cmd;
    cmd.format("createNode myLocator -name \"^1s\";", locatorName);
    MGlobal::executeCommand(cmd);

	// create connection between deltaMushNode and locator
    MStatus status = MGlobal::executeCommand(MString("connectAttr ")
        + deltaMushNodeName + ".message "
        + locatorName + ".deformerMessage");

    if (!status) 
    {
        MGlobal::displayError("Failed to connect deformer to locator!");
		return MS::kFailure;
    }
    MGlobal::displayInfo("Locator node created");
	return MStatus::kSuccess;
}


std::vector<MBoundingBox> buildFaceAABBs(MDagPath& meshObj)
{
    std::vector<MBoundingBox> faceBoxes;

    MStatus status;
    MItMeshPolygon faceIt(meshObj, MObject::kNullObj, &status);
    if (!status) return faceBoxes;

    faceBoxes.reserve(faceIt.count());

    for (; !faceIt.isDone(); faceIt.next())
    {
        int faceIndex = faceIt.index(&status);

        MPointArray pts;
        faceIt.getPoints(pts, MSpace::kObject);

        // Build bounding box:
        MBoundingBox bbox;
        for (unsigned int i = 0; i < pts.length(); ++i)
            bbox.expand(pts[i]);

        faceBoxes.push_back( bbox);
    }

    return faceBoxes;
}


MStatus MyPluginCmd::collisonTest(MDagPath& dagPath0, MDagPath& dagPath1)
{
    MStatus status;

    // Optionally verify they are mesh shapes
    MFnMesh fnMesh0(dagPath0, &status);
    if (!status) {
        MGlobal::displayError("First selected object is not a mesh.");
        return MS::kFailure;
    }
    
    // create an empty mesh data container
    MFnMeshData meshDataFn;
    MObject newMeshData = meshDataFn.create(&status);
    if (!status) { MGlobal::displayError("Failed to create mesh data"); return  MS::kFailure;; }

    // copy the source mesh geometry into mesh data (no DAG node)
    // copy(src, dst) -- src can be shape or mesh data, dst is mesh data.
    MObject smoothMeshObj = fnMesh0.copy(fnMesh0.object(), newMeshData);
    
    // work on the copy using MFnMesh and iterators
    MFnMesh fnMesh1(newMeshData, &status);
    if (!status) { MGlobal::displayError("Failed attach to copy mesh data"); return  MS::kFailure;; }
    

    //MFnMesh fnMesh1(dagPath1, &status);




    double smoothingFactor = 0.5;

    unsigned int vertexCount = fnMesh1.numVertices();
    MPointArray currentPoints;
    fnMesh1.getPoints(currentPoints, MSpace::kObject);

    for (int i = 0; i < 20; ++i)
    {
        MPointArray updatedPoints = currentPoints;// create copy of the points

        MItMeshVertex itVert(dagPath0, MObject::kNullObj, &status);
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
                avgPos = avgPos / connectedVerticesIndices.length();

                MVector diff = avgPos - currentPoints[idx];
                updatedPoints[idx] += diff * smoothingFactor;
            }

            itVert.next();
        }
        currentPoints = updatedPoints;
    }
    // Apply new positions
    status = fnMesh1.setPoints(currentPoints, MSpace::kObject);
    MGlobal::displayInfo("Mesh modified successfully.");


    MMeshIntersector intersector;;
    status = intersector.create(smoothMeshObj);
    if (!status) {
		MGlobal::displayError("Failed to create MMeshIntersector.");
    }
    MItMeshVertex vertIt(dagPath0, MObject::kNullObj, &status);
    if (!status) {
        // Handle error
    }
    MItMeshPolygon faceIt(dagPath0, MObject::kNullObj, &status);
    MFloatVectorArray normals;
	fnMesh0.getVertexNormals(false, normals, MSpace::kObject);

    MPointArray Points;
    fnMesh1.getPoints(Points, MSpace::kObject);
    //allIntersections flags
    MMeshIsectAccelParams accelParams;
    accelParams = fnMesh0.autoUniformGridParams();
    bool sortHits = false;
    float tolerance = 0.0001f;
    MFloatPointArray hitPoints;
    MFloatArray hitRayParams;
    MIntArray hitFaces;
    MIntArray hitTriangles;
    MFloatArray hitBary1;
    MFloatArray hitBary2;
    MFloatPoint hitpoint;
    double bias = 1e-4;
    MPoint closePoint;
    MVector closeNormal;
	std::vector<MPoint> collisionPoints;
    std::vector<MPointArray> collisionFaces;
    for (; !vertIt.isDone(); vertIt.next()) {
        int idx = vertIt.index(&status);
        // e.g. get position
        MPoint pos = vertIt.position(MSpace::kObject, &status);
        // do your work



		MFloatVector normal = -normals[idx];
        auto raySource = MFloatPoint(pos[0], pos[1], pos[2], 1.0);

        auto rayDir = MFloatVector(normal[0], normal[1], normal[2]);
        bool hit = fnMesh1.allIntersections(raySource, rayDir, NULL, NULL, false, MSpace::kObject, 99999, false, &accelParams, false,
            hitPoints, &hitRayParams, &hitFaces, &hitTriangles, &hitBary1, &hitBary2, 0.000001f);


        
        if (hit)
        {
            MPointOnMesh closest;
            auto f = intersector.getClosestPoint(pos, closest);
            if (f != MS::kSuccess)
            {
                continue;
            }
			closePoint = closest.getPoint();
			closeNormal = closest.getNormal();
            auto delta = pos - closePoint;
            //collision check with dot product
            auto angle = delta * closeNormal;
            if (angle < 0)
            {
                //MGlobal::displayInfo("Collision");
                collisionPoints.push_back(pos);
                collidedVertices.insert(idx);

                    
            }
        }
        
    }
    MGlobal::executeCommand("createNode myLocator");
    MGlobal::displayInfo("Collision test executed.");
    MGlobal::displayInfo(std::to_string(collisionPoints.size()).c_str());
}


MStatus  MyPluginCmd::collisonTest2(MDagPath& dagPath0, MDagPath& dagPath1)
{
    MStatus status;

    // Optionally verify they are mesh shapes
    MFnMesh fnMesh0(dagPath0, &status);
    if (!status) {
        MGlobal::displayError("First selected object is not a mesh.");
        return MS::kFailure;
    }


    MFnMesh fnMesh1(dagPath1, &status);

    MItMeshVertex vertIt(dagPath0, MObject::kNullObj, &status);
    if (!status) {
        // Handle error
    }
    MItMeshPolygon faceIt(dagPath0, MObject::kNullObj, &status);
    MFloatVectorArray normals;
    fnMesh0.getVertexNormals(false, normals, MSpace::kObject);


    //allIntersections flags
    MMeshIsectAccelParams accelParams;
    accelParams = fnMesh0.autoUniformGridParams();
    bool sortHits = false;
    float tolerance = 0.0001f;
    MFloatPointArray hitPoints;
    MFloatArray hitRayParams;
    MIntArray hitFaces;
    MIntArray hitTriangles;
    MFloatArray hitBary1;
    MFloatArray hitBary2;
    double bias = 1e-4;
    MPoint closePoint;
    MVector closeNormal;
    std::vector<MPoint> collisionPoints;
    std::vector<MPointArray> collisionFaces;
    for (; !vertIt.isDone(); vertIt.next()) {
        int idx = vertIt.index(&status);

        if(collidedVertices.find(idx) != collidedVertices.end())
        {
            continue;
		}

        // e.g. get position
        MPoint pos = vertIt.position(MSpace::kObject, &status);
        // do your work

        MFloatVector normal = -normals[idx];
        auto raySource = MFloatPoint(pos[0] + normal[0] * bias, pos[1] + normal[0] * bias, pos[2] + normal[0] * bias, 1.0);

        auto rayDir = MFloatVector(normal[0], normal[1], normal[2]);
        bool hit = fnMesh1.allIntersections(raySource, rayDir, NULL, NULL, false, MSpace::kObject, 99999, false, &accelParams, false,
            hitPoints, &hitRayParams, &hitFaces, &hitTriangles, &hitBary1, &hitBary2, 0.000001f);

        if (hit)
        {

            fnMesh1.getClosestPointAndNormal(pos, closePoint, closeNormal, MSpace::kObject, NULL, NULL);

            auto delta = pos - closePoint;
            //collision check with dot product
            auto angle = delta * closeNormal;
            if (angle < 0)
            {
                //MGlobal::displayInfo("Collision");
                collisionPoints.push_back(pos);

            }

        }
    }

    MGlobal::displayInfo("Collision test executed.");
    MGlobal::displayInfo(std::to_string(collisionPoints.size()).c_str());

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




