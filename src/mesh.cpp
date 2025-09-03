#include "framework.h"
#include "mesh.h"
#include <maya/MString.h>

MeshHandler::MeshHandler(const MDagPath& dagpath) : m_dagPath(dagpath), m_fnMesh(dagpath)
{
    m_fnMesh.getPoints(m_vertices, MSpace::kObject);
    //m_fnMesh.getNormals(m_normals, MSpace::kObject);
    m_fnMesh.getVertexNormals(m_normals);
    m_fnMesh.getVertices(m_verticesCounts, m_verticesIndices);
    initConnected();
}

MeshHandler::MeshHandler(const MeshHandler& other)
    : m_dagPath(other.m_dagPath),
    m_vertices(other.m_vertices),
    m_normals(other.m_normals),
    m_verticesCounts(other.m_verticesCounts),
    m_verticesIndices(other.m_verticesIndices),
    m_fnMesh(other.m_dagPath) // reinitialize from copied dagPath
{
    initConnected();
}


MeshHandler::MeshHandler(const MObject& mesh) : m_fnMesh (mesh)
{
    m_fnMesh.getVertices(m_verticesCounts, m_verticesIndices);

    m_fnMesh.getTangents(m_tangents);

    m_fnMesh.getBinormals(m_binormals);


    int se = m_verticesCounts.length();
    int te = m_verticesIndices.length();
    initConnected();


}

std::set<int> MeshHandler::getConnectedVertices(int index)
{
    return connected[index];
}


void MeshHandler::initConnected()
{
    connected.resize(m_fnMesh.numVertices());

    int indexOffset = 0;
    for (unsigned int f = 0; f < m_verticesCounts.length(); ++f)
    {
        int count = m_verticesCounts[f];
        for (int i = 0; i < count; ++i)
        {
            int v0 = m_verticesIndices[indexOffset + i];
            int v1 = m_verticesIndices[indexOffset + (i + 1) % count];
            connected[v0].insert(v1);
            connected[v1].insert(v0);
        }
        indexOffset += count;
    }
}
void MeshHandler::resetNormals()
{
    m_normals.setLength(m_vertices.length());
    for (unsigned int i = 0; i < m_normals.length(); ++i) {
        m_normals[i] = MFloatVector(0.0f, 0.0f, 0.0f);
    }

}

PolyData MeshHandler::getPolydata( size_t index, int count)
{
    std::vector<MPoint> polyVerts;
    std::vector<int> polyIndices;
    for (int i = 0; i < count; ++i) {
        int vi = m_verticesIndices[index + i];
        polyVerts.push_back(m_vertices[vi]);
        polyIndices.push_back(vi);
    }
    PolyData polydat
    {
        polyIndices,
        polyVerts
    };
    return polydat;
}


MVector MeshHandler::computePolyNormal(std::vector<MPoint>& polyVerts, int count)
{

    MVector faceNormal(0.0, 0.0, 0.0);
    for (int i = 0; i < count; ++i) {
        MPoint& curr = polyVerts[i];
        MPoint& next = polyVerts[(i + 1) % count];
        faceNormal.x += (curr.y - next.y) * (curr.z + next.z);
        faceNormal.y += (curr.z - next.z) * (curr.x + next.x);
        faceNormal.z += (curr.x - next.x) * (curr.y + next.y);
    }
    faceNormal.normalize();

    return faceNormal;
}


MPoint MeshHandler::getNextPoint(int index)
{
    MStatus status;
    MPoint p1;
    status = m_fnMesh.getPoint(index, p1, MSpace::kWorld);
    if (status != MS::kSuccess) 
    {
        MGlobal::displayError("Failed to get connected vertex position.");
    }
    return p1;
}

void MeshHandler::calcVerticesNormal(std::vector<int>& polyIndices, MVector faceNormal, int count)
{
    for (int i = 0; i < count; ++i) 
    {
        int vi = polyIndices[i];
        m_normals[vi] += faceNormal;
    }
}


void MeshHandler::normalizeNormals()
{
    for (unsigned int i = 0; i < m_normals.length(); ++i) {
        m_normals[i].normalize();
    }
}


void MeshHandler::recalculateNormals()
{
    resetNormals();
    size_t index = 0;
    for (unsigned int i = 0; i < m_verticesCounts.length(); ++i) {
        int count = m_verticesCounts[i];
        PolyData polyData = getPolydata(index, count);    
        MVector faceNormal = computePolyNormal(polyData.second, count);
        calcVerticesNormal(polyData.first, faceNormal, count);
        index += count;
    }
    normalizeNormals();
}

MeshHandler& MeshHandler::operator=(const MeshHandler& other) {
    if (this != &other) {
        m_dagPath = other.m_dagPath;
        m_vertices = other.m_vertices;
        m_normals = other.m_normals;
        m_verticesCounts = other.m_verticesCounts;
        m_verticesIndices = other.m_verticesIndices;

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

void MeshHandler::info()
{
    MString msg;
    msg += "Vertices: ";      msg += (int)m_vertices.length();
    msg += " | Normals: ";    msg += (int)m_normals.length();
    msg += " | VertexCounts: "; msg += (int)m_verticesCounts.length();
    msg += " | VertexIndices: "; msg += (int)m_verticesIndices.length();
    msg += " | Binormals: "; msg += (int)m_binormals.length();
    msg += " | Tangents: "; msg += (int)m_tangents.length();

    MGlobal::displayInfo(msg);
}


void MeshHandler::updateMesh() {
    m_fnMesh.setPoints(m_vertices, MSpace::kObject);
    m_fnMesh.setNormals(m_normals, MSpace::kObject);
}


MeshHandler MeshHandler::createCopy()
{
    MStatus stattus;
    // Create a new mesh data container
    MFnMeshData meshDataFn;
    MObject newMeshData = meshDataFn.create(&stattus);

    MObject meshObj = m_fnMesh.object();

    if (meshObj.isNull())
    {
        MGlobal::displayError("Object is null");
    }

    MObject copiedMeshObj = m_fnMesh.copy(meshObj, newMeshData, &stattus);


    if (copiedMeshObj.isNull())
    {
        MGlobal::displayError("Copy Object is null");
    }

    MeshHandler copyMeshHandeler = MeshHandler (copiedMeshObj);
    copyMeshHandeler.setNormals(this->m_normals);
    copyMeshHandeler.setVertices(this->m_vertices);
    copyMeshHandeler.info();
    MGlobal::displayInfo("Copy successfully.");
    return copyMeshHandeler;
}


