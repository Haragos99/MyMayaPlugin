#pragma once
#include "framework.h"
#include "mesh.h"


class DeltaMush {
public:
	DeltaMush(MDagPath& dagPath);
	MeshHandler smoothMesh(MeshHandler mesh,int iterations); 
	void CalculateDelta();

private:
	MeshHandler m_mesh;
	std::vector<MPoint> deltas;


};