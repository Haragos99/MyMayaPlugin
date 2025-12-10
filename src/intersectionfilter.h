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
		std::set<int> filterFirstIntersections(const MPointArray& original, MeshHandler& target);
		std::set<int> filterDefromIntersections(const MPointArray& original, MeshHandler& target, std::set<int> filtered);
		void separateFilteredData(Collison& data);
		void initFilteredData(MeshHandler& mesh);

private:
	// create a distence change filte
	MMeshIntersector m_intersector;
	MObject meshDataObj;
	MFnMesh meshFn;
	std::unordered_map<int, std::pair<int, int>> edIDX;
	std::unordered_map<int, MIntArray> facesIDX;
	std::set<int> vertexesIDX;
};