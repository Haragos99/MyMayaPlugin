#include "pch.h"
#include "mesh.h"


MeshHandler::MeshHandler(const MDagPath& dagpath) : m_dagPath(dagpath), m_fnMesh(dagpath)
{
    m_fnMesh.getPoints(m_vertices, MSpace::kObject);
    m_fnMesh.getNormals(m_normals, MSpace::kObject);
    m_fnMesh.getVertices(m_verticesCounts, m_verticesIndices);
}

MeshHandler::MeshHandler(const MeshHandler& other)
    : m_dagPath(other.m_dagPath),
    m_vertices(other.m_vertices),
    m_normals(other.m_normals),
    m_verticesCounts(other.m_verticesCounts),
    m_verticesIndices(other.m_verticesIndices),
    m_fnMesh(other.m_dagPath) // reinitialize from copied dagPath
{}





void MeshHandler::recalculateNormals()
{
    m_normals.setLength(m_vertices.length());
    for (unsigned int i = 0; i < m_normals.length(); ++i) {
        m_normals[i] = MFloatVector(0.0f, 0.0f, 0.0f);
    }

    size_t index = 0;
    for (unsigned int poly = 0; poly < m_verticesCounts.length(); ++poly) {
        int count = m_verticesCounts[poly];

        // Get polygon vertices
        std::vector<MPoint> polyVerts;
        std::vector<int> polyIndices;

        for (int i = 0; i < count; ++i) {
            int vi = m_verticesIndices[index + i];
            polyVerts.push_back(m_vertices[vi]);
            polyIndices.push_back(vi);
        }

        // Compute polygon normal (assume convex polygon, use Newell method or simple cross product)
        MVector faceNormal(0.0, 0.0, 0.0);
        for (int i = 0; i < count; ++i) {
            const MPoint& curr = polyVerts[i];
            const MPoint& next = polyVerts[(i + 1) % count];
            faceNormal.x += (curr.y - next.y) * (curr.z + next.z);
            faceNormal.y += (curr.z - next.z) * (curr.x + next.x);
            faceNormal.z += (curr.x - next.x) * (curr.y + next.y);
        }
        faceNormal.normalize();

        // Add face normal to each vertex normal
        for (int i = 0; i < count; ++i) {
            int vi = polyIndices[i];
            m_normals[vi] += faceNormal;
        }

        index += count;
    }

    // Normalize vertex normals
    for (unsigned int i = 0; i < m_normals.length(); ++i) {
        m_normals[i].normalize();
    }
}

MeshHandler& MeshHandler::operator=(const MeshHandler& other) {
    if (this != &other) {
        m_dagPath = other.m_dagPath;
        m_vertices = other.m_vertices;
        m_normals = other.m_normals;
        m_verticesCounts = other.m_verticesCounts;
        m_verticesIndices = other.m_verticesIndices;

        // Do NOT assign m_fnMesh or m_dagPath
    }
    return *this;
}



std::shared_ptr<MItMeshVertex> MeshHandler::getVertexIterator(MStatus* status ) const
{
    return std::make_shared<MItMeshVertex>(m_dagPath, MObject::kNullObj, status);
}
std::shared_ptr<MItMeshPolygon> MeshHandler::getPolygonIterator(MStatus* status ) const
{
    return std::make_shared<MItMeshPolygon>(m_dagPath, MObject::kNullObj, status);
}
std::shared_ptr<MItMeshEdge> MeshHandler::getEdgeIterator(MStatus* status ) const
{
    return std::make_shared<MItMeshEdge>(m_dagPath, MObject::kNullObj, status);
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


