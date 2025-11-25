#pragma once
#include "framework.h"
#include "mesh.h"
#include "collison.h"

class DeltaMush {
public:
	DeltaMush(MDagPath& dagPath);
	MeshHandler smoothMesh(MeshHandler mesh,int iterations); 
	void CalculateDelta();
	void CalculateDeformation();
	void CCDDeformation();
	void debugCCD(int itertion, MPointArray points);
	void test(MPointArray points);
	void improvedDM(MPointArray points);
	void setDeltaMushFactor(float factor) { deltaMushFactor = factor / 100; }
	void setStrength(float str) { strength = str; }
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
	float strength;
	int smoothIterion;
	MeshHandler m_mesh;
	MeshHandler m_smooth;
	std::vector<MPoint> deltas;
	std::set<int> m_filteredIndices;
	MMatrix initMatrix(MPoint point, MVector normal, MVector tangent, MVector bitangent);
	std::vector<MPoint> scaleDeltas(std::vector<MPoint> originalDeltas, float smooth);
	void smoothCollidedVertices(std::set<int>& collededVertexes);
	void projectPointToPlane(const MPoint& P, const MVector& N, MPoint& Q);
};