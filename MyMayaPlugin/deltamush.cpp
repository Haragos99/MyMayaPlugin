#include "pch.h"
#include "deltamush.h"


std::shared_ptr<MeshHandler>DeltaMush::smoothMesh(MeshHandler mesh,int iterations)
{
	double smoothingFactor = 0.5;
	MPointArray currentPoints = mesh.getVertices();
	for (int i = 0; i < iterations; ++i)
	{
		MPointArray updatedPoints = currentPoints;// create copy of the points
		auto itVert = mesh.getVertexIterator();
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
	return;
}


void DeltaMush::CalculateDelta()
{

}