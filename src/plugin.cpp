#include "plugin.h"
#include "deltamushnode.h"
#include <maya/MFnLambertShader.h>
#include <maya/MFnSet.h>
#include "debugdraweroverride.h"
MCallbackIdArray MyPluginCmd::g_callbackIds;
std::shared_ptr<DeltaMush> MyPluginCmd::deltamush;


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
    
    //collisonTest();
	runDeltaMush();
    return MStatus::kSuccess;
}



MStatus MyPluginCmd::runDeltaMush()
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
    //MObject node = fnDep.create(nodeType, &status);
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


    MGlobal::executeCommand("createNode myLocator");


    MGlobal::displayInfo("Mesh modified successfully.");
	return MS::kSuccess;
}



MStatus MyPluginCmd::collisonTest()
{
    
    MStatus status;
    // Get current selection list
    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);

    // Check for exactly two items
    if (selection.length() == 0) {
        MGlobal::displayError("Please select mesh object");
        return MS::kFailure;
    }
    // Get DagPaths for each selected item
    MDagPath dagPath0;
    selection.getDagPath(0, dagPath0);


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
        // e.g. get position
        MPoint pos = vertIt.position(MSpace::kObject, &status);
        // do your work

		MFloatVector normal = -normals[idx];
        auto raySource = MFloatPoint(pos[0]+ normal[0] * bias, pos[1] + normal[0] * bias, pos[2]+ normal[0]* bias, 1.0);

        auto rayDir = MFloatVector(normal[0], normal[1], normal[2]);
        bool hit = fnMesh1.allIntersections(raySource, rayDir, NULL, NULL, false, MSpace::kObject, 99999, false, &accelParams, false,
            hitPoints, &hitRayParams, &hitFaces, &hitTriangles, &hitBary1, &hitBary2, 0.000001f);

        if (hit)
        {
            //fnMesh0.getClosestPointAndNormal(pos, closePoint, closeNormal, MSpace::kWorld, NULL, NULL);
        
            //collision check with dot product
            //auto angle = delta * closeNormal;
            //MGlobal::displayInfo(std::to_string(hitFaces.length()).c_str());

            fnMesh1.getClosestPointAndNormal(pos, closePoint, closeNormal, MSpace::kObject, NULL, NULL);

            auto delta = pos - closePoint;
            //collision check with dot product
            auto angle = delta * closeNormal;
            if (angle < 0)
            {
                //MGlobal::displayInfo("Collision");
                collisionPoints.push_back(pos);

                for (int i = 0; i < 0; ++i)
                {
                    int perv;
                    faceIt.setIndex(hitFaces[i], perv);

                    MIntArray faceVertices;
                    faceIt.getVertices(faceVertices);
                    MPointArray points;
                    faceIt.getPoints(points, MSpace::kObject);

                    collisionFaces.push_back(points);
                }

                    
            }

            /*
            if (hitFaces.length() > 1)
            {
                for(int i = 0; i < hitFaces.length(); ++i)
                {
                    MPoint newPoint = MPoint(hitPoints[i]);
					delta = pos - newPoint;
                    fnMesh1.getClosestNormal(newPoint, closeNormal, MSpace::kWorld, NULL);
                    auto angle = delta * closeNormal;
                    if (angle < 0)
                    {
                        int perv;
						faceIt.setIndex(hitFaces[i], perv);

						MIntArray faceVertices;
						faceIt.getVertices(faceVertices);
                        MPointArray points;
                        faceIt.getPoints(points, MSpace::kWorld);

                        

                        
                        
                        MGlobal::displayInfo("The Collision on face: " + MString() + hitFaces[i]);
                        collisionPoints.push_back(pos);
						collisionFaces.push_back(points);
                        
                       // MGlobal::displayInfo("The Collision on face: " + MString() + hitFaces[i]);
                        //collisionPoints.push_back(pos);
                        break;
                    }
				}
            }
            */
           
        }
    }
    MGlobal::executeCommand("createNode myLocator");
	MyLocatorDrawOverride::CDpoints = collisionPoints;
	MyLocatorDrawOverride::CDfaces = collisionFaces;
    MGlobal::displayInfo("Collision test executed.");
    MGlobal::displayInfo(std::to_string(collisionPoints.size()).c_str());
}


bool MyPluginCmd::isPointInPlane(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c)
{
    // Define a small tolerance value (epsilon)
    const double epsilon = 1e-9;
    MVector pa = p - a;
    MVector pb = p - b;
    MVector pc = p - c;
    return std::abs(mixed(pa, pb, pc)) <= epsilon;
}


double MyPluginCmd::mixed(MVector& a, MVector& b, MVector& c)
{
    auto cross = b ^ c;
    return a * cross;
}



bool MyPluginCmd::isPointInTriangle(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c)
{
    if (!isPointInPlane(p, a, b, c))
    {
        return false;
    }

    MVector ba = b - a;
    MVector ca = c - a;
    MVector pa = p - a;

    const auto normDir = ba ^ ca;
    if ((normDir * (ba ^ pa)) < 0)
    {
        return false;
    }

    MVector cb = c - b;
    MVector pb = p - b;
    if ((normDir * (cb ^ pb)) < 0)
    {
        return false;
    }

    MVector pc = p - c;
    MVector ac = a - c;
    if ((normDir * (ac ^ pc)) < 0)
    {
        return false;
    }
    return true;
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




