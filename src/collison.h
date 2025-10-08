#pragma once
#include "framework.h"
#include "mesh.h"




struct Delta {

public:
	MPoint getDeltaPoint(MMatrix&& C) {
		MPoint v = deltavector;
		v[0] *= toi;
		v[1] *= toi;
		v[2] *= toi;
		auto d = C * v;
		return MPoint(d[0], d[1], d[2]);
	}
	float toi;
	bool isCollied;
	Delta(MPoint deltavector_, float toi_, bool isCollied_) : deltavector(deltavector_), toi(toi_), isCollied(isCollied_) {}
private:
	MPoint deltavector;
};

class Collison {

public:
	Collison(std::vector<MPoint> v);
	void init(std::vector<MPoint> v);

	bool collisondetec(MeshHandler& mesh, MeshHandler& smooth);

	float getAlfa() { return alfa; }
	void setAlfa(float newalfa) { alfa = newalfa; }


private:
	Eigen::Vector3f toEigenVec(const MPoint v) {
		return Eigen::Vector3f(v[0], v[1], v[2]);
	}
	void setRestToi(float newtoi);
	void setMeshTio(int vertexIdx, MeshHandler& mesh);
	void setSmalest(int vvertexIdx, int f, int edegs, int edegs2, MeshHandler& mesh);
	void restCollied();
	std::vector<Delta> deltas;
	Eigen::Vector3f err = Eigen::Vector3f(-1, -1, -1);  // Error bounds
	float tmax;
	float tmaxiter;
	float tolerance;
	float mc;
	std::vector<float> tois;
	float smallestTio;
	float prevTio;
	float alfa;
};