#pragma once
#include "pch.h"
#include "mesh.h"


class DeltaMush {
public:
	DeltaMush(MDagPath& dagPath);
	MeshHandler smoothMesh(MeshHandler mesh,int iterations); 
	void CalculateDelta();

private:
	MeshHandler m_mesh;


};