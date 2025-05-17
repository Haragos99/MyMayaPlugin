#pragma once
#include "pch.h"
#include "mesh.h"


class DeltaMush {
	DeltaMush(std::shared_ptr<MeshHandler> mesh);
	std::shared_ptr<MeshHandler> smoothMesh(MeshHandler mesh,int iterations); 
	void CalculateDelta();

private:
	std::shared_ptr<MeshHandler> m_mesh;


};