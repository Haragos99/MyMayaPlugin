#include "deltamush.h"
#include <chrono>
#include "intersectionfilter.h"
#include <tbb/parallel_for.h>

DeltaMush::DeltaMush(MDagPath& dagPath) : m_mesh(dagPath)
{
	smoothIterion = 20;
	deltaMushFactor = 1.0f;
	strength = 0.5f;
	m_mesh.collectVerticesNearPoint(MPoint(0, 0, 0),1.8);
}

MeshHandler DeltaMush::smoothMesh(MeshHandler mesh,int iterations)
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

void DeltaMush::CalculateDelta()
{
	deltas.clear();
	m_smooth = smoothMesh(m_mesh, smoothIterion);
	m_smooth.recalculateNormals();
	m_smooth.info();

	MStatus status;
	auto& originalPoints = m_mesh.getVertices();
	auto& smoothpoints = m_smooth.getVertices();
	auto& normals = m_smooth.getNormals();
	for (int vertIndex = 0; vertIndex < smoothpoints.length(); ++vertIndex)
	{
		MPoint point = smoothpoints[vertIndex];

		// Retrieve vertex normal
		MVector normal = normals[vertIndex];

		normal.normalize();

		int neighborIdx = *m_smooth.getConnectedVertices(vertIndex).begin();

		MPoint neighbor = smoothpoints[neighborIdx];

		MVector edge = neighbor - point;

		// Compute tangent vector 
		MVector tangent = edge - (edge * normal) * normal;
		tangent.normalize();

		// Compute bitangent as cross product of normal and tangent
		MVector bitangent = tangent ^ normal;
		bitangent.normalize();

		// Construct projection matrix R
		MMatrix R = initMatrix(point, normal, tangent, bitangent);

		MMatrix R_inv = R.inverse();

		// Transform the position vector into tangent space
		MPoint delta = R_inv * originalPoints[vertIndex];
		deltas.push_back(delta);
	}
	MGlobal::displayInfo("Delta successfully.");
	MGlobal::displayInfo(std::to_string(deltas.size()).c_str());

	IntersectionFilter filter(m_smooth);
	m_filteredIndices = filter.filterFirstIntersections(m_mesh.getVertices(), m_mesh);
	MGlobal::displayInfo("Defult filter successfully.");
}

MMatrix DeltaMush::initMatrix(MPoint point, MVector normal, MVector tangent, MVector bitangent)
{
	MMatrix M;
	M[0][0] = tangent.x;   M[0][1] = normal.x;   M[0][2] = bitangent.x;   M[0][3] = point.x;
	M[1][0] = tangent.y;   M[1][1] = normal.y;   M[1][2] = bitangent.y;   M[1][3] = point.y;
	M[2][0] = tangent.z;   M[2][1] = normal.z;   M[2][2] = bitangent.z;   M[2][3] = point.z;
	M[3][0] = 0.0;         M[3][1] = 0.0;        M[3][2] = 0.0;        M[3][3] = 1.0;

	return M;
}

std::vector<MPoint> DeltaMush::scaleDeltas(std::vector<MPoint> originalDeltas, float smooth)
{
	std::to_string(smooth);
	MGlobal::displayInfo(std::to_string(smooth).c_str());

	for (auto& delta : originalDeltas)
	{
		delta[0] *= smooth;
		delta[1] *= smooth;
		delta[2] *= smooth;
	}
	return originalDeltas;
}

void DeltaMush::debugCCD(int itertion, MPointArray points)
{
	m_collisonData.clear();
	m_mesh.setVertices(points);
	MGlobal::displayInfo("Start Debugg");
	CalculateDeformation();
	Collison collison = Collison(deltas);
	auto smooth = smoothMesh(m_mesh, smoothIterion);
	IntersectionFilter filter(smooth);
	filter.filterDefromIntersections(m_mesh.getVertices(), m_mesh, m_filteredIndices);
	filter.initFilteredData(m_mesh);
	filter.separateFilteredData(collison);

	int i = 0;
	while(collison.collisondetec(m_mesh, m_smooth, m_collisonData) && i < itertion)
	{
		MGlobal::displayInfo("Iter: ");

		CCDDeformation();
		float alfa = collison.getAlfa();
		int percent = alfa * 100;
		collison.setAlfa(0);
		++i;
	}
	smoothCollidedVertices(collison.vertexes);
	m_mesh.updateMesh();
}

void DeltaMush::CalculateDeformation()
{
	m_smooth = smoothMesh(m_mesh, smoothIterion);
	m_smooth.recalculateNormals();

	auto newDeltas = scaleDeltas(deltas, deltaMushFactor);

	MStatus status;	
	auto& smoothPoints = m_smooth.getVertices();
	auto& smoothNormals = m_smooth.getNormals();
	MPointArray deformedPoints;
	for (int vertIndex = 0; vertIndex < smoothPoints.length(); ++vertIndex)
	{
		MPoint point = smoothPoints[vertIndex];
		// Retrieve vertex normal
		MVector normal = smoothNormals[vertIndex];
		normal.normalize();

		int neighborIdx = *m_smooth.getConnectedVertices(vertIndex).begin();

		MPoint neighbor = smoothPoints[neighborIdx];

		MVector edge = neighbor - point;

		// Compute tangent vector 
		MVector tangent = edge - (edge * normal) * normal;
		tangent.normalize();

		// Compute bitangent as cross product of normal and tangent
		MVector bitangent = tangent ^ normal;
		bitangent.normalize();

		// Construct projection matrix C
		MMatrix C = initMatrix(point, normal, tangent, bitangent);
		m_mesh.setMatrix(vertIndex, C);
		MPoint defomedpoint = C * newDeltas[vertIndex];
		deformedPoints.append(defomedpoint);
	}
	m_mesh.setVertices(deformedPoints);
}

void DeltaMush::test(MPointArray points)
{
	MColorArray colors;
	colors.setLength(points.length());;
	m_mesh.addcolor(colors);
	m_mesh.setVertices(points);
	CalculateDeformation();
	m_mesh.updateMesh();
}


void DeltaMush::improvedDM(MPointArray points)
{
	m_mesh.setVertices(points);
	MGlobal::displayInfo("Start");
	CalculateDeformation();
	m_collisonData.clear();

	auto start = std::chrono::high_resolution_clock::now();
	MeshHandler smooth = smoothMesh(m_mesh, smoothIterion);
	IntersectionFilter filter(smooth);
	filter.filterDefromIntersections(m_mesh.getVertices(), m_mesh, m_filteredIndices);
	m_collisonData.collidedFacesIdx = filter.fIndices;
	Collison collison = Collison(deltas);
	filter.initFilteredData(m_mesh);
	filter.separateFilteredData(collison);
	
	MGlobal::displayInfo(std::to_string(m_collisonData.intersected.size()).c_str());
	while(collison.collisondetec(m_mesh, m_smooth, m_collisonData))
	{
		CCDDeformation();
		float alfa = collison.getAlfa();
		int percent = alfa * 100;
		collison.setAlfa(0);
	}
	
	m_collisonData.collidedAllVertecesIdx = collison.vertexes;
	smoothCollidedVertices(collison.vertexes);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	MGlobal::displayInfo(MString("Execution time: ") + duration + " ms");
	m_mesh.updateMesh();
}

void DeltaMush::CCDDeformation()
{
	m_smooth.recalculateNormals();
	MStatus status;
	auto& smoothPoints = m_smooth.getVertices();
	auto& smoothNormals = m_smooth.getNormals();
	MPointArray deformedPoints;
	for (int vertIndex = 0; vertIndex < smoothPoints.length(); ++vertIndex)
	{
		MPoint point = smoothPoints[vertIndex];
		// Retrieve vertex normal
		MVector normal = smoothNormals[vertIndex];
		normal.normalize();

		int neighborIdx = *m_smooth.getConnectedVertices(vertIndex).begin();

		MPoint neighbor = smoothPoints[neighborIdx];

		MVector edge = neighbor - point;

		// Compute tangent vector 
		MVector tangent = edge - (edge * normal) * normal;
		tangent.normalize();

		// Compute bitangent as cross product of normal and tangent
		MVector bitangent = tangent ^ normal;
		bitangent.normalize();

		// Construct projection matrix C
		MMatrix C = initMatrix(point, normal, tangent, bitangent);
		m_mesh.setMatrix(vertIndex, C);
		MPoint defomedpoint = C * deltas[vertIndex];
		deformedPoints.append(defomedpoint);
	}
	m_mesh.setVertices(deformedPoints);
}

// TODO: reftor this function not Delta mush resposible for projection
void DeltaMush::projectPointToPlane(const MPoint& P, const MVector& N, MPoint& Q)
{
	MVector norm = N;
	norm.normalize();

	// distance between the pointand the plane
	float d = (P - Q) * norm;

	// projected point
	Q = Q - (d * norm);
}

void DeltaMush::smoothCollidedVertices(std::set<int>& collidedVertices)
{
	const auto& normals = m_mesh.getNormals();

	// collect neighbouring vertices (connected to collided ones, but not themselves collided)
	std::set<int> neighbours;
	for (int v : collidedVertices) {
		for (int vi : m_mesh.getConnectedVertices(v)) {
			if (collidedVertices.find(vi) == collidedVertices.end()) {
				neighbours.insert(vi);
			}
		}
	}

	// lambda that encapsulates the smoothing logic
	auto smoothSet = [&](const std::set<int>& verts, int iterations) 
	{
		for (int it = 0; it < iterations; ++it) 
		{
			for (int v : verts) 
			{
				MVector actualPoint = m_mesh.getPoint(v);
				MVector avg;   
				int count = 0;

				for (int vi : m_mesh.getConnectedVertices(v)) {
					auto point = m_mesh.getPoint(vi);
					// project neighbour point to the plane defined by the current vertex
					projectPointToPlane(actualPoint, normals[v], point);
					avg += point;
					++count;
				}
				if (count == 0)
				{
					continue; // safety: avoid division by zero
				}

				avg /= count;
				actualPoint += strength * (avg - actualPoint);
				m_mesh.setPoint(v, actualPoint);
			}
		}
	};

	// apply smoothing: collided vertices 2 iterion, neighbours 1 iterion
	smoothSet(collidedVertices, 2);
	smoothSet(neighbours, 1);
}

void DeltaMush::setLocalToWorldMatrix(const MMatrix& matrix)
{ 
	m_worldMatrix = matrix; 
}

MeshData* DeltaMush::createMeshData()
{
	return new MeshData(
		deltaMushFactor, 
		deltas, m_mesh.getEdgesIndices(), 
		m_mesh.getFacesIndices(), 
		m_mesh.getMatricesC(), 
		m_collisonData, 
		m_mesh.getVertices(), 
		m_smooth.getVertices(),
		m_worldMatrix
	);
}