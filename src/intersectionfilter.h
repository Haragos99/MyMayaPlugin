#include "mesh.h"
#include "collison.h"
class IntersectionFilter {
	public:
		IntersectionFilter(MeshHandler& target);
		std::vector<MPoint> clalculateIntersections(const MPointArray& original, MeshHandler& target);
		std::unordered_map<int, MIntArray> faceIndices;
		std::set<int> vertexIndices;
		std::set<int> edgeIndices;
		std::set<int> findSelfCollidingTriangles(MeshHandler& target);
		std::set<int> clalculateIntersections(MeshHandler& original, MeshHandler& smooth);

private:

	// create a distence change filte
	MMeshIntersector m_intersector;
	MObject meshDataObj;
	MFnMesh meshFn;
	bool areFacesAdjacent(std::shared_ptr<MItMeshPolygon> polyIter, int faceA, int faceB);
	bool triangleIntersectsTriangle(const MPoint& v0, const MPoint& v1, const MPoint& v2,
		const MPoint& u0, const MPoint& u1, const MPoint& u2,
		double epsilon = 1e-6);
	bool isPointInTriangle(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c);
	double mixed(MVector& a, MVector& b, MVector& c);
	bool isPointInPlane(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c);
	bool areFacesNearbyNeighbors(const MFnMesh& meshFn, int faceA, int faceB, double threshold = 1e-6);

};