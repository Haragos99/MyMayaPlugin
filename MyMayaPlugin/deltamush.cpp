#include "pch.h"
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
	mesh.updateMesh();
	return mesh;
	//return std::make_shared<MeshHandler>(mesh);

}


void DeltaMush::CalculateDelta()
{
	smoothMesh(m_mesh,10);
}