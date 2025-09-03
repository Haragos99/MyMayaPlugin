#include "deltamush.h"




DeltaMush::DeltaMush(MDagPath& dagPath) : m_mesh(dagPath)
{

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
	//mesh.updateMesh();
	return mesh.createCopy();
	//return std::make_shared<MeshHandler>(mesh);

}


void DeltaMush::CalculateDelta()
{
	deltas.clear();

	auto smooth = smoothMesh(m_mesh,10);
	smooth.updateMesh();
	smooth.info();
	MStatus status;
	MPoint p1(0.0, 0.0, 0.0);
	MPoint p2(1.0, 1.0, 1.0);
	drawLine(p1, p2);
	auto original = m_mesh.getVertices();
	auto points = smooth.getVertices();
	auto normals = smooth.getNormals();
	for (int vertIndex = 0; vertIndex < points.length(); ++vertIndex)
	{
		MGlobal::displayInfo("O-null");
		MPoint point = points[vertIndex];
		if (status != MS::kSuccess)
		{
			MGlobal::displayError("Failed to get vertex position.");
			continue;
		}
		// Retrieve vertex normal
		MVector normal = normals[vertIndex];

		if (status != MS::kSuccess)
		{
			MGlobal::displayError("Failed to get vertex normal.");
			continue;
		}
		normal.normalize();

		auto verexes = smooth.getConnectedVertices(vertIndex);

		MPoint p1 = points[*verexes.begin()];

		MVector edge = p1 - point;

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
		MPoint delta = R_inv * original[vertIndex];
		deltas.push_back(delta);
		std::string d = std::to_string(delta[0]) + ", " + std::to_string(delta[1]) + ", " + std::to_string(delta[2]);
		MGlobal::displayInfo(d.c_str());

	}
	MGlobal::displayInfo("Delta successfully.");
	MGlobal::displayInfo(std::to_string(deltas.size()).c_str());
	//m_mesh.recalculateNormals();
	//m_mesh.updateMesh();
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

void DeltaMush::CalculateDeformation()
{
	MGlobal::displayInfo("Adsdsad ");
	auto Anormals = m_mesh.getNormals();
	auto smooth = smoothMesh(m_mesh, 10);
	smooth.updateMesh();
	smooth.info();
	MStatus status;
	
	auto points = smooth.getVertices();
	auto normals = smooth.getNormals();

	for (int i = 0; i < normals.length(); i++)
	{
		MGlobal::displayInfo(MString("S Normal: ") +
			normals[i].x + ", " +
			normals[i].y + ", " +
			normals[i].z + MString("! O Normal: ")+
			Anormals[i].x + ", " +
			Anormals[i].y + ", " +
			Anormals[i].z 
		);

	}

	MPointArray deformedPoints;
	for (int vertIndex = 0; vertIndex < points.length(); ++vertIndex)
	{
		MPoint point = points[vertIndex];
		if (status != MS::kSuccess)
		{
			MGlobal::displayError("Failed to get vertex position.");
			continue;
		}
		// Retrieve vertex normal
		MVector normal = normals[vertIndex];

		if (status != MS::kSuccess)
		{
			MGlobal::displayError("Failed to get vertex normal.");
			continue;
		}
		normal.normalize();

		MIntArray connectedVertices;
		auto verexes = smooth.getConnectedVertices(vertIndex);

		MPoint p1 = points[*verexes.begin()];

		MVector edge = p1 - point;

		// Compute tangent vector 
		MVector tangent = edge - (edge * normal) * normal;
		tangent.normalize();

		// Compute bitangent as cross product of normal and tangent
		MVector bitangent = tangent ^ normal;
		bitangent.normalize();

		// Construct projection matrix C
		MMatrix C = initMatrix(point, normal, tangent, bitangent);
		MPoint defompoint = C * deltas[vertIndex];
		deformedPoints.append(defompoint);
		

	}


	
	
	MGlobal::displayInfo(std::to_string(deformedPoints.length()).c_str());
	m_mesh.setVertices(deformedPoints);
	m_mesh.updateMesh();
}

// Implementation of line drawing using legacy OpenGL
void DeltaMush::drawLine(const MPoint& p1, const MPoint& p2)
{
	MGlobal::displayInfo("Trscs: ");
}


void DeltaMush::move()
{
	/*
	MPointArray currentPoints = m_mesh.getVertices();
	MStatus status;
	auto vertIter = m_mesh.getVertexIterator(&status);
	for (; !vertIter->isDone(); vertIter->next())
	{
		MPoint avgPos(0.1, 0.1, 0.1);
		int idx = vertIter->index();
		currentPoints[idx] += avgPos;
	}
	m_mesh.setVertices(currentPoints);
	m_mesh.updateMesh();
	*/
	auto smooth = smoothMesh(m_mesh, 1);
	m_mesh = smooth;
	m_mesh.updateMesh();
}


void DeltaMush::test(MPointArray points)
{
	m_mesh.info();
	MGlobal::displayInfo("Test k ");
	m_mesh.setVertices(points);
	m_mesh.updateMesh();
	auto smooth = smoothMesh(m_mesh, 1);
	m_mesh.setVertices(smooth.getVertices());
	//m_mesh.updateMesh();
}