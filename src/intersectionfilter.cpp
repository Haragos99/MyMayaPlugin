#include "intersectionfilter.h"
#include <thread> 
#include <tbb/parallel_for.h>
#include <numeric>
#include <fstream>

IntersectionFilter::IntersectionFilter(MeshHandler& target)
	: m_intersector()
{
	target.updateMesh();
	// Initialize the intersector with the mesh object
	auto meshObj = target.getMeshObject();

	MStatus status;

	// Create an in-memory mesh data container (not visible in the scene)
	MFnMeshData meshDataFn;
	meshDataObj = meshDataFn.create(&status);
	meshFn;
	MObject smoothMeshObj = meshFn.copy(meshObj, meshDataObj, &status);
	auto dataPoints = target.getVertices();
	meshFn.setPoints(dataPoints, MSpace::kObject);

	status = m_intersector.create(smoothMeshObj);
	if (!status) {
		MGlobal::displayError("Failed to create MMeshIntersector.");
	}

}



std::set<int> IntersectionFilter::filterDefromIntersections(const MPointArray& original, MeshHandler& target, std::set<int> filtered)
{
	std::set<int> filteredIndices = clalculateIntersections(original, target, filtered);
	for (const auto& idx : filteredIndices)
	{
		vertexIndices.insert(idx);
		auto connect = target.getConnectedVertices(idx);
		for (const auto& connectedIdx : connect)
		{
			vertexIndices.insert(connectedIdx);
		}
	}


	for (const auto& idx : vertexIndices)
	{
		MIntArray faceID = target.getConnectedVertexFaces(idx);
		for (int i : faceID)
		{
			fIndices.insert(i);
		}
	}


	MGlobal::displayInfo(
		MString("Intersections calculated: ")
		+ MString() + std::to_string(vertexIndices.size()).c_str()
	);

	return filteredIndices;
}


std::set<int> IntersectionFilter::filterFirstIntersections(const MPointArray& original, MeshHandler& target)
{
	std::set<int> empty;
	std::set<int> filterVertexIndices = clalculateIntersections(original, target, empty);
	MGlobal::displayInfo(
		MString("Intersections calculated: ")
		+ MString() + std::to_string(filterVertexIndices.size()).c_str()
	);

	return filterVertexIndices;
}

std::set<int> IntersectionFilter::clalculateIntersections(const MPointArray& original, MeshHandler& target, std::set<int> filtered)
{
	target.updateMesh();
	//target.recalculateNormals();
	double epsilon = 1e-6;
	auto normlas = target.getMeshNormals();
	MMeshIsectAccelParams accelParams;
	accelParams = target.getIntersectParameters();
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
	std::vector<MPoint> coints;
	std::set<int> collisionPoints;
	auto vertIt = target.getVertexIterator(nullptr);
	for (; !vertIt->isDone(); vertIt->next())
	{

		int i = vertIt->index();

		if (filtered.find(i) != filtered.end())
		{
			continue;
		}

		MPoint p1 = original[i];    
		MFloatVector normal = -normlas[i];
		auto raySource = MFloatPoint(p1[0], p1[1], p1[2], 1.0);

		auto rayDir = MFloatVector(normal[0], normal[1], normal[2]);
		bool hit = meshFn.allIntersections(raySource, rayDir, NULL, NULL, false, MSpace::kObject, 99999, false, &accelParams, false,
			hitPoints, &hitRayParams, &hitFaces, &hitTriangles, &hitBary1, &hitBary2, 0.000001f);

		if (hit)
		{
			MPointOnMesh closest;
			auto f = m_intersector.getClosestPoint(p1, closest);
			if (f != MS::kSuccess)
			{
				continue;
			}
			MPoint closestPoint = closest.getPoint();
			MFloatVector closestnormal = closest.getNormal();

			auto delta = p1 - closestPoint;
			//collision check with dot product
			auto angle = delta * closestnormal;
			if (angle <= 0)
			{
				collisionPoints.insert(i);
				coints.push_back(p1);
			}
		}
	}
	return collisionPoints;
}



// TODO: DELETE THIS FUNCTION LATER
std::set<int> IntersectionFilter::clalculateIntersections(MeshHandler& original, MeshHandler& smooth)
{
	original.updateMesh();
	original.recalculateNormals();
	auto normlas = original.getNormals();
	auto& points = original.getVertices();
	//allIntersections flags
	MMeshIsectAccelParams accelParams;
	accelParams = original.getIntersectParameters();
	smooth.updateMesh();
	bool sortHits = false;
	float tolerance = 0.0001f;
	MFloatPointArray hitPoints;
	MFloatArray hitRayParams;
	MIntArray hitFaces;
	MIntArray hitTriangles;
	MFloatArray hitBary1;
	MFloatArray hitBary2;
	auto& fnMesh1 = smooth.getMeshFunction();

	MPoint closePoint;
	MVector closeNormal;
	std::set<int> collisionPoints;
	for (int idx = 0; idx < points.length(); ++idx) {
		// e.g. get position
		MPoint pos = points[idx];
		// do your work

		MFloatVector normal = -normlas[idx];
		auto raySource = MFloatPoint(pos[0], pos[1], pos[2], 1.0);

		auto rayDir = MFloatVector(normal[0], normal[1], normal[2]);
		bool hit = fnMesh1.allIntersections(raySource, rayDir, NULL, NULL, false, MSpace::kObject, 99999, false, &accelParams, false,
			hitPoints, &hitRayParams, &hitFaces, &hitTriangles, &hitBary1, &hitBary2, 0.000001f);

		if (hit)
		{
			fnMesh1.getClosestPointAndNormal(pos, closePoint, closeNormal, MSpace::kWorld, NULL, NULL);
			auto delta = pos - closePoint;
			//collision check with dot product
			auto angle = delta * closeNormal;
			if (angle < 0)
			{
				MGlobal::displayInfo("ACollision");
				collisionPoints.insert(idx);
			}
		}
	}
	MGlobal::displayInfo("Collision");
	return collisionPoints;
}












