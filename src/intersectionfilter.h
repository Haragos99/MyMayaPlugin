#include "mesh.h"
#include "collison.h"
class IntersectionFilter {
	public:
		IntersectionFilter(MeshHandler& target);
		std::set<int> clalculateIntersections(const MPointArray& original, MeshHandler& target, std::set<int> filtered);
		std::unordered_map<int, MIntArray> faceIndices;
		std::set<int> fIndices;
		std::set<int> vertexIndices;
		std::set<int> edgeIndices;
		std::set<int> clalculateIntersections(MeshHandler& original, MeshHandler& smooth);
		std::set<int> filterFirstIntersections(const MPointArray& original, MeshHandler& target);
		std::set<int> filterDefromIntersections(const MPointArray& original, MeshHandler& target, std::set<int> filtered);

private:

	// create a distence change filte
	MMeshIntersector m_intersector;
	MObject meshDataObj;
	MFnMesh meshFn;
};