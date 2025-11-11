#include "mesh.h"
#include "collison.h"
class IntersectionFilter {
	public:
		IntersectionFilter(MeshHandler& target);
		void clalculateIntersections(const MPointArray& original, const MPointArray& smooth, MeshHandler& target, MeshHandler& sm, CollisonData& data);
		std::unordered_map<int, MIntArray> faceIndices;
		std::set<int> vertexIndices;
		std::set<int> edgeIndices;
		std::set<int>findSelfCollidingTriangles(MeshHandler& target);
		std::set<int> clalculateIntersections(MeshHandler& original, MeshHandler& smooth);

private:

	// create a distence change filte
	MMeshIntersector m_intersector;
	MObject meshDataObj;

};