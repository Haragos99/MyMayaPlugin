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
	return mesh;
	//return std::make_shared<MeshHandler>(mesh);

}


void DeltaMush::CalculateDelta()
{
	auto smooth = smoothMesh(m_mesh,10);
	MStatus status;
	auto vertIter = smooth.getVertexIterator(&status);
	for (; !vertIter->isDone(); vertIter->next())
	{
		int vertIndex = vertIter->index();
		MPoint p = vertIter->position(MSpace::kWorld, &status);
		if (status != MS::kSuccess) 
		{
			MGlobal::displayError("Failed to get vertex position.");
			continue;
		}
		// Retrieve vertex normal
		MVector normal;
		status = vertIter->getNormal(normal, MSpace::kWorld);
		if (status != MS::kSuccess) 
		{
			MGlobal::displayError("Failed to get vertex normal.");
			continue;
		}
		normal.normalize();

		MIntArray connectedVertices;
		status = vertIter->getConnectedVertices(connectedVertices);
		if (status != MS::kSuccess || connectedVertices.length() == 0) {
			MGlobal::displayError("Failed to get connected vertices.");
			continue;
		}

		MPoint p1 = smooth.getNextPoint(connectedVertices[0]);

		MVector edge = p1 - p;

		// Compute tangent vector by removing the component along the normal
		MVector tangent = edge - (edge * normal) * normal;
		tangent.normalize();

		// Compute bitangent as cross product of normal and tangent
		MVector bitangent = normal ^ tangent;
		bitangent.normalize();

		// Construct transformation matrix R
		MMatrix R;
		R[0][0] = tangent.x;   R[0][1] = bitangent.x;   R[0][2] = normal.x;   R[0][3] = p.x;
		R[1][0] = tangent.y;   R[1][1] = bitangent.y;   R[1][2] = normal.y;   R[1][3] = p.y;
		R[2][0] = tangent.z;   R[2][1] = bitangent.z;   R[2][2] = normal.z;   R[2][3] = p.z;
		R[3][0] = 0.0;         R[3][1] = 0.0;           R[3][2] = 0.0;        R[3][3] = 1.0;

		// Invert the transformation matrix
		MMatrix R_inv = R.inverse();

		// Transform the position vector into local space
		MPoint delta = p * R_inv;
		deltas.push_back(delta);

	}
	MGlobal::displayInfo("Delta successfully.");




	//m_mesh.recalculateNormals();
	//m_mesh.updateMesh();
}