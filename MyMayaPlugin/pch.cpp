

#include "pch.h"


static Eigen::Vector3f Utilty::toEigenVec(const MVector& v) { return  Eigen::Vector3f(v[0], v[1], v[2]); }

static MVector Utilty::cross(MPoint& p1, MPoint& p2)
{
	double x = p1.y * p2.z - p1.z * p2.y;
	double y = p1.z * p2.x - p1.x * p2.z;
	double z = p1.x * p2.y - p1.y * p2.x;
	return MVector(x, y, z);
}


