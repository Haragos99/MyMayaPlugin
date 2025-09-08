#pragma once
#include "framework.h"
#include "mesh.h"


class DeltaMush {
public:
	DeltaMush(MDagPath& dagPath);
	MeshHandler smoothMesh(MeshHandler mesh,int iterations); 
	void CalculateDelta();
	void CalculateDeformation();
	void move();
	void test(MPointArray points);
private:
	MeshHandler m_mesh;
	std::vector<MPoint> deltas;
	MMatrix initMatrix(MPoint point, MVector normal, MVector tangent, MVector bitangent);
	void drawLine(const MPoint& p1, const MPoint& p2);
};