#pragma once
#include "pch.h"



struct MeshData {

};


class MeshHandler {
public:
    
    MeshHandler(const MDagPath& dagpath);
    MeshHandler() = default;
    MeshHandler(const MeshHandler& other);

    // Accessors
    const MPointArray& getVertices() const;
    const MFloatVectorArray& getNormals() const;
    const MIntArray& getVerticesCounts() const;
    const MIntArray& getVerticesIndices() const;

    // Modifiers
    void setVertices(const MPointArray& points);
    void setNormals(const MFloatVectorArray& normals);
    
    void recalculateNormals();

    MeshHandler& operator=(const MeshHandler& other);
    // Iterators
    std::shared_ptr<MItMeshVertex> getVertexIterator(MStatus* status ) const;
    std::shared_ptr<MItMeshPolygon> getPolygonIterator(MStatus* status) const;
    std::shared_ptr<MItMeshEdge> getEdgeIterator(MStatus* status) const;

    // Utility Methods
    void updateMesh(); // Apply changes to the mesh
    ~MeshHandler(){}
    
private:
    MDagPath  m_dagPath;
    MFnMesh m_fnMesh;
    MPointArray m_vertices;
    MFloatVectorArray m_normals;
    MIntArray m_verticesCounts; //The number of vertices for each polygon.
    MIntArray m_verticesIndices; //The indices of all vertices in the mesh 

};
