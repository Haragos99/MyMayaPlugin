#include "intersectionfilter.h"
#include <thread> 
#include <tbb/parallel_for.h>
#include <numeric>
#include <fstream>

IntersectionFilter::IntersectionFilter(MeshHandler& target)
	: m_intersector()
{
	target.updateMesh();
	auto normals = target.computePerVertexNormals();
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

/// <summary>
/// Computes filtered indices based on intersections between an input point array and a target mesh. 
/// The method expands the result to include connected vertices and faces, updates the object's vertex and face index sets,
/// and logs the number of vertices found.
/// </summary>
/// <param name="original:">Array of points (MPointArray) used to compute intersections against the target mesh.</param>
/// <param name="target:">MeshHandler providing mesh connectivity queries (e.g., connected vertices and faces) used to expand affected indices.</param>
/// <param name="filtered:">Initial set of indices to seed the intersection calculation; used as input to determine which indices to process.</param>
/// <returnsindices of the intersections vertices.</returns>
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
			MIntArray conID = target.getConnectedFaces(i);
			for (int j : conID)
			{
				fIndices.insert(j);
				MIntArray bonID = target.getConnectedFaces(j);
				for (int k : bonID)
				{
					fIndices.insert(k);
				}

			}
		}
	}

	MGlobal::displayInfo(
		MString("Intersections calculated: ")
		+ MString() + std::to_string(vertexIndices.size()).c_str()
	);

	return filteredIndices;
}

/// <summary>
/// filter the unnesarzy intersections by returning the indices of the vertices that are intersecting with the target mesh.
/// </summary>
/// <param name="original:"></param>
/// <param name="target:"></param>
/// <returns>indices of the intersections vertices</returns>
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

/// <summary>
/// return the indices of the vertices that are intersecting with the target mesh. 
/// It also takes a set of filtered indices to avoid checking them again.
/// </summary>
/// <param name="original:">Points of the original mesh</param>
/// <param name="target:">meshhandel of the original mesh</param>
/// <param name="filtered:">the filtered verteces idx </param>s
/// <returns>indices of the intersections vertices indices</returns>
std::set<int> IntersectionFilter::clalculateIntersections(const MPointArray& original, MeshHandler& target, std::set<int> filtered)
{
	target.updateMesh();
	target.recalculateNormals();
	double epsilon = 1e-6;
	auto normlas = target.getMeshNormals();
	double bias = 1e-4;
	std::vector<MPoint> collisionPoints;
	std::set<int> collisionPointsIdx;
	auto vertIt = target.getVertexIterator(nullptr);
	for (; !vertIt->isDone(); vertIt->next())
	{
		int i = vertIt->index();

		if (filtered.find(i) != filtered.end())
		{
			continue;
		}

		MPoint orignalPoint = original[i];    
		MFloatVector normal = -normlas[i];
		MPointOnMesh closest;
		auto f = m_intersector.getClosestPoint(orignalPoint, closest);
		if (f != MS::kSuccess)
		{
			continue;
		}
		MPoint closestPoint = closest.getPoint();
		MFloatVector closestnormal = closest.getNormal();
		auto delta = orignalPoint - closestPoint;

		//collision check with dot product
		auto angle = delta * closestnormal;
		if (angle <= 0)
		{
			collisionPointsIdx.insert(i);
			collisionPoints.push_back(orignalPoint);
		}
	}

	return collisionPointsIdx;
}

void IntersectionFilter::separateFilteredData(Collison& data)
{
	data.vertexesIDX = vertexesIDX;
	data.facesIDX = facesIDX;
	data.edgesIDX = edIDX;
}


void IntersectionFilter::initFilteredData(MeshHandler& mesh)
{
	auto& faceData = mesh.getFacesData();
	auto& faces = mesh.getFacesIndices();
	auto& edgesIDX = mesh.getEdgesIndices();

	for (int face : fIndices)
	{
		auto faceVerts = faces.at(face);
		facesIDX[face] = faceVerts;
		auto& edges = faceData[face].edgesIndices;
		auto& verteses = faceData[face].vertexIndices;

		vertexesIDX.insert(verteses.begin(), verteses.end());

		for (int edgeIdx : edges)
		{
			edIDX[edgeIdx] = edgesIDX.at(edgeIdx);
		}
	}
}