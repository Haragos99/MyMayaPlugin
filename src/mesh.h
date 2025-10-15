#pragma once
#include "framework.h"
#include <set>


using PolyData = std::pair<std::vector<int>, std::vector<MPoint>>;
struct MeshData {

};


class MeshHandler {
public:
    
    MeshHandler(const MDagPath& dagpath);
    MeshHandler (const MObject& mesh);
    MeshHandler() = default;
    MeshHandler(const MeshHandler& other);

    // Accessors
    const MPointArray& getVertices() const;
    const MFloatVectorArray& getNormals() const;
    const MIntArray& getVerticesCounts() const;
    const MIntArray& getVerticesIndices() const;
    const std::unordered_map<int, MIntArray>& getFacesIndices() const;
    const std::unordered_map<int, std::pair<int, int>>& getEdgesIndices() const;



    // Modifiers
    void setVertices(const MPointArray& points);
    void setNormals(const MFloatVectorArray& normals);
    
    void addcolor(MColorArray colors);

    void recalculateNormals();

    std::set<int> getConnectedVertices(int index);

    MeshHandler& operator=(const MeshHandler& other);
    // Iterators
    std::shared_ptr<MItMeshVertex> getVertexIterator(MStatus* status ) const;
    std::shared_ptr<MItMeshPolygon> getPolygonIterator(MStatus* status) const;
    std::shared_ptr<MItMeshEdge> getEdgeIterator(MStatus* status) const;

    void collectVerticesNearPoint(const MPoint& origin, double threshold = 0.1);

    MPoint getPoint(int index);

    void setPoint(int index, MPoint newPoint);

    MMatrix getMatrixC(int vertexIdx);

    void resetNormals();

    void normalizeNormals();

    MPoint getNextPoint(int index);
    void info();
    MeshHandler createCopy();
    void setMatrix(int idx, const MMatrix& C);
    // Utility Methods
    void updateMesh(); // Apply changes to the mesh
    ~MeshHandler(){}
    
private:
    
    PolyData getPolydata(size_t index, int count);
    MVector computePolyNormal(std::vector<MPoint>& polyVerts, int count);
    void calcVerticesNormal(std::vector<int>& polyIndices, MVector faceNormal, int count);
    void initConnected();
    void initFaces();
    void initEdges();
    MDagPath  m_dagPath;
    MFnMesh m_fnMesh;
    MPointArray m_vertices;
    MFloatVectorArray m_normals;
    MFloatVectorArray m_binormals;
    MFloatVectorArray m_tangents;
    MIntArray m_verticesCounts; //The number of vertices for each polygon.
    MIntArray m_verticesIndices; //The indices of all vertices in the mesh
    std::vector<std::set<int>> m_connected;
    std::unordered_map<int, MIntArray> m_faceToVerts;
    std::unordered_map<int, std::pair<int,int>> m_edgeToVerts;
    std::set<int> nearbyVertices;
    std::vector<MMatrix> m_matrcesC;// refactor maybe
};
