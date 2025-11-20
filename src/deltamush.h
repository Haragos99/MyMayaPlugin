#pragma once
#include "framework.h"
#include "mesh.h"
#include "collison.h"

class DeltaMush {
public:
	DeltaMush(MDagPath& dagPath);
	MeshHandler smoothMesh(MeshHandler mesh,int iterations, bool saveSmooth = true); 
	void CalculateDelta();
	void CalculateDeformation();
	void CCDDeformation();
	void move();
	void debugCCD(int itertion, MPointArray points);
	void test(MPointArray points);
	void improvedDM(MPointArray points);
	void setDeltaMushFactor(float factor) { deltaMushFactor = factor/100; }
	float getDeltaMushFactor() { return deltaMushFactor; }
	MPointArray getPoints() { return m_mesh.getVertices(); }
	CollisonData& getCollisonData() { return m_collisonData; }
	void updateMesh() { m_mesh.updateMesh(); }	
	MeshHandler& getMeshHandler() { return m_mesh; }
	MeshHandler& getSmoothMeshHandler() { return m_smooth; }
	std::vector<MPoint> getDeltas() { return deltas; }
private:
	CollisonData m_collisonData;
	float deltaMushFactor;
	int smoothIterion;
	MeshHandler m_mesh;
	MeshHandler m_smooth;
	std::vector<MPoint> deltas;
	std::set<int> m_filteredIndices;
	MMatrix initMatrix(MPoint point, MVector normal, MVector tangent, MVector bitangent);
	std::vector<MPoint> scaleDeltas(std::vector<MPoint> originalDeltas, float smooth);
	void drawLine(const MPoint& p1, const MPoint& p2);
};