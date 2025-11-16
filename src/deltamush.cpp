#include "deltamush.h"
#include <chrono>
#include "intersectionfilter.h"
#include <tbb/parallel_for.h>


DeltaMush::DeltaMush(MDagPath& dagPath) : m_mesh(dagPath)
{
	smoothIterion = 20;
	deltaMushFactor = 1.0f;
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
	m_smooth = mesh;
	return mesh;
}

void DeltaMush::CalculateDelta()
{
	deltas.clear();
	auto smooth = smoothMesh(m_mesh, smoothIterion);
	smooth.recalculateNormals();
	smooth.info();

	MStatus status;
	auto& originalPoints = m_mesh.getVertices();
	auto& smoothpoints = smooth.getVertices();
	auto& normals = smooth.getNormals();
	for (int vertIndex = 0; vertIndex < smoothpoints.length(); ++vertIndex)
	{
		MPoint point = smoothpoints[vertIndex];

		// Retrieve vertex normal
		MVector normal = normals[vertIndex];

		normal.normalize();

		int neighborIdx = *smooth.getConnectedVertices(vertIndex).begin();

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
	m_mesh.setVertices(points);
	MGlobal::displayInfo("Start Debugg");
	CalculateDeformation();
	Collison collison = Collison(deltas);
	auto smooth = smoothMesh(m_mesh, smoothIterion);
	smooth.recalculateNormals();
	int i = 0;
	while(i < itertion)
	{
		MGlobal::displayInfo("Iter");
		//collison.collisondetec(m_mesh, smooth);
		//CalculateDeformation();
		float alfa = collison.getAlfa();
		int percent = alfa * 100;
		collison.setAlfa(0);
		i++;
	}
}


void DeltaMush::CalculateDeformation()
{
	auto smooth = smoothMesh(m_mesh, smoothIterion);
	smooth.recalculateNormals();

	auto newDeltas = scaleDeltas(deltas, deltaMushFactor);

	MStatus status;	
	auto& smoothPoints = smooth.getVertices();
	auto& smoothNormals = smooth.getNormals();
	MPointArray deformedPoints;
	for (int vertIndex = 0; vertIndex < smoothPoints.length(); ++vertIndex)
	{
		MPoint point = smoothPoints[vertIndex];
		// Retrieve vertex normal
		MVector normal = smoothNormals[vertIndex];
		normal.normalize();

		int neighborIdx = *smooth.getConnectedVertices(vertIndex).begin();

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
	//m_mesh.updateMesh();
}

// Implementation of line drawing using legacy OpenGL
void DeltaMush::drawLine(const MPoint& p1, const MPoint& p2)
{

}


void DeltaMush::move()
{
	auto smooth = smoothMesh(m_mesh, 1);
	m_mesh = smooth;
	
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
	filter.clalculateIntersections(m_mesh.getVertices(), m_mesh);
	m_collisonData.collidedVertecesIdx = filter.vertexIndices;

	Collison collison = Collison(deltas);
	
	collison.edgesIDX = m_mesh.getNearbyEdges();
	collison.facesIDX = m_mesh.getNearbyFaces();
	collison.vertexesIDX = m_mesh.getNearbyVertices();
	//m_collisonData.collidedFacesIdx = filter.findSelfCollidingTriangles(m_mesh);
	//m_collisonData.collidedVertecesIdx = filter.clalculateIntersections(m_mesh,m_smooth);

	/*
	MGlobal::displayInfo(std::to_string(m_collisonData.intersected.size()).c_str());
	while(collison.collisondetec(m_mesh, m_smooth, m_collisonData))
	{
		//collison.collisondetec(m_mesh, smooth);
		CCDDeformation();
		float alfa = collison.getAlfa();
		int percent = alfa * 100;
		collison.setAlfa(0);
	}
	*/
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