#pragma once
#include "framework.h"
#include "mesh.h"


class DeltaMush {
public:
	DeltaMush(MDagPath& dagPath);
	MeshHandler smoothMesh(MeshHandler mesh,int iterations); 
	void CalculateDelta();
	void CalculateDeformation();
	void CCDDeformation();
	void move();
	void debugCCD(int itertion, MPointArray points);
	void test(MPointArray points);
	void improvedDM(MPointArray points);
	void setDeltaMushFactor(float factor) { deltaMushFactor = factor/100; }
	MPointArray getPoints() { return m_mesh.getVertices(); }
	void updateMesh() { m_mesh.updateMesh(); }	
private:
	float deltaMushFactor;
	int smoothIterion;
	MeshHandler m_mesh;
	MeshHandler m_smooth;
	std::vector<MPoint> deltas;
	MMatrix initMatrix(MPoint point, MVector normal, MVector tangent, MVector bitangent);
	std::vector<MPoint> scaleDeltas(std::vector<MPoint> originalDeltas, float smooth);
	void drawLine(const MPoint& p1, const MPoint& p2);
};