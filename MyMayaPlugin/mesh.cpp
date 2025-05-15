#include "pch.h"
#include "mesh.h"


MeshHandler::MeshHandler(const MObject& mesh) : m_mesh(mesh), m_fnMesh(mesh)
{
    m_fnMesh.getPoints(m_vertices, MSpace::kObject);
    m_fnMesh.getNormals(m_normals, MSpace::kObject);
    m_fnMesh.getVertices(m_verticesCounts, m_verticesIndices);
}



std::shared_ptr<MItMeshVertex> MeshHandler::getVertexIterator(MStatus* status = nullptr) const
{
    return std::make_shared<MItMeshVertex>(m_mesh, MObject::kNullObj, status);
}
std::shared_ptr<MItMeshPolygon> MeshHandler::getPolygonIterator(MStatus* status = nullptr) const
{
    return std::make_shared<MItMeshPolygon>(m_mesh, MObject::kNullObj, status);
}
std::shared_ptr<MItMeshEdge> MeshHandler::getEdgeIterator(MStatus* status = nullptr) const
{
    return std::make_shared<MItMeshEdge>(m_mesh, MObject::kNullObj, status);
}

const MPointArray& MeshHandler::getVertices() const 
{
    return m_vertices;
}

const MFloatVectorArray& MeshHandler::getNormals() const
{
    return m_normals;
}

const MIntArray& MeshHandler::getVerticesCounts() const
{
    return m_verticesCounts;
}

const MIntArray& MeshHandler::getVerticesIndices() const
{
    return m_verticesIndices;
}


void MeshHandler::setVertices(const MPointArray& points) 
{
    m_vertices = points;
}

void MeshHandler::setNormals(const MFloatVectorArray& normals)
{
    m_normals = normals;
}

void MeshHandler::updateMesh() {
    m_fnMesh.setPoints(m_vertices, MSpace::kObject);
    m_fnMesh.setNormals(m_normals, MSpace::kObject);
}


