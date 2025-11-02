#include "mesh.h"
#include "collison.h"
class IntersectionFilter {
	public:
		IntersectionFilter(MeshHandler& target);
		void clalculateIntersections(const MPointArray& original, const MPointArray& smooth, MeshHandler& target, MeshHandler& sm, CollisonData& data);
		std::unordered_map<int, MIntArray> faceIndices;
		std::set<int> vertexIndices;
		std::set<int> edgeIndices;

private:

	// create a distence change filte
	MMeshIntersector m_intersector;
	MObject meshDataObj;

};