#include "framework.h"
#include "mesh.h"
#include <maya/MString.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnSet.h>

MeshHandler::MeshHandler(const MDagPath& dagpath) : m_dagPath(dagpath), m_fnMesh(dagpath)
{
    m_fnMesh.getPoints(m_vertices, MSpace::kObject);
    m_fnMesh.getVertexNormals(false,m_normals);
    m_fnMesh.getVertices(m_verticesCounts, m_verticesIndices);
    initConnected();
    initFaces();
    initEdges();
    m_matrcesC.resize(m_vertices.length());
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
    initFaces();
    initEdges();
    m_matrcesC.resize(m_vertices.length());
    m_matrcesC = other.m_matrcesC;
}


void MeshHandler::addcolor(MColorArray colors)
{
    // TODO: It kills maya
    MColor color = MColor(1);
    //m_fnMesh.setVertexColor(color, 0);
    //m_fnMesh.setFaceColor(color, 0);
    MStatus status;
     // Create Lambert shader
   

}

void MeshHandler::collectVerticesNearPoint(const MPoint& origin, double threshold)
{
    for(int i = 0; i < m_vertices.length(); ++i)
    {
		MPoint p = m_vertices[i];
        if (p.distanceTo(origin) <= threshold)
        {
            nearbyVertices.insert(i);
        }
    }


    for(auto face : m_faceToVerts)
    {
        auto idx = face.second;
        for(int i = 0; i < idx.length(); ++i)
        {
            MPoint p = m_vertices[idx[i]];
            if (p.distanceTo(origin) <= threshold)
            {
				m_nearby_faceToVerts[face.first] = idx;
				break;
            }
		}
    }


    for(auto edge : m_edgeToVerts)
    {
        auto idxs = edge.second;
		int first_edge = idxs.first;
		int second_edge = idxs.second;
		MPoint p1 = m_vertices[first_edge];
		MPoint p2 = m_vertices[second_edge];

        if (p1.distanceTo(origin) <= threshold || p2.distanceTo(origin) <= threshold)
        {
            m_nearby_edgeToVerts[edge.first] = idxs;
        }
	}

    MGlobal::displayInfo(MString("Found ") + (int)nearbyVertices.size() + " vertices near point. "+"Faces: "+ (int)m_nearby_faceToVerts.size() + " Edges: " + (int)m_nearby_edgeToVerts.size());
}

MeshHandler::MeshHandler(const MObject& mesh) : m_fnMesh (mesh)
{
    m_fnMesh.getVertices(m_verticesCounts, m_verticesIndices);

    m_fnMesh.getTangents(m_tangents);

    m_fnMesh.getBinormals(m_binormals);

    int se = m_verticesCounts.length();
    int te = m_verticesIndices.length();
    initConnected();
    initFaces();
    initEdges();
    m_matrcesC.resize(m_vertices.length());
}

std::set<int>& MeshHandler::getConnectedVertices(int index)
{
    return m_connected[index];
}


void MeshHandler::initConectedVertexToFace()
{
	auto vertexIt = getVertexIterator(nullptr);
    for (;!vertexIt->isDone(); vertexIt->next())
    {
		int vIdx = vertexIt->index();
		MIntArray connectedFaces;
		vertexIt->getConnectedFaces(connectedFaces);
		m_vertexToFaces[vIdx] = connectedFaces;
    }
}


void MeshHandler::initConnected()
{
    m_connected.resize(m_fnMesh.numVertices());

    int indexOffset = 0;
    for (unsigned int f = 0; f < m_verticesCounts.length(); ++f)
    {
        int count = m_verticesCounts[f];
        for (int i = 0; i < count; ++i)
        {
            int v0 = m_verticesIndices[indexOffset + i];
            int v1 = m_verticesIndices[indexOffset + (i + 1) % count];
            m_connected[v0].insert(v1);
            m_connected[v1].insert(v0);
        }
        indexOffset += count;
    }
}


// Create a new MObject representing the mesh with updated vertices
MObject MeshHandler::getMeshObject()
{
    MStatus status;

    // Create an in-memory mesh data container (not visible in the scene)
    MObject sourceMeshObj = m_fnMesh.object();
    return sourceMeshObj;
}


void MeshHandler::initFaces()
{
    MStatus status;
    auto polyIt = getPolygonIterator(&status);

    for (; !polyIt->isDone(); polyIt->next()) 
    {
		FaceData faceData;
        int fIdx = polyIt->index(&status);
		faceData.faceIndex = fIdx;
        MIntArray faceVerts;
        polyIt->getVertices(faceVerts);
		faceData.vertexIndices = faceVerts;

        MIntArray faceEdges;
		polyIt->getEdges(faceEdges);
		faceData.edgesIndices = faceEdges;

        m_faceToVerts[fIdx] = faceVerts;
        MIntArray cfaces;
        polyIt->getConnectedFaces(cfaces);
		m_connected_face[fIdx] = cfaces;
    }

	initConectedVertexToFace();
}
void MeshHandler::initEdges()
{
    MStatus status;
    auto edgeIt = getEdgeIterator(&status);
    for (; !edgeIt->isDone(); edgeIt->next()) 
    {
        int edgeIndex = edgeIt->index(&status);
        int v0 = edgeIt->index(0, &status);
        int v1 = edgeIt->index(1, &status);
        m_edgeToVerts[edgeIndex] = { v0 ,v1 };
    }
}
void MeshHandler::setMatrix(int idx, const MMatrix& C)
{
    m_matrcesC[idx] = C;
}


void MeshHandler::resetNormals()
{
    m_normals.setLength(m_vertices.length());
    for (unsigned int i = 0; i < m_normals.length(); ++i) 
    {
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
    for (int i = 0; i < m_normals.length(); ++i) 
    {
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
    if (this != &other) 
    {
        m_dagPath = other.m_dagPath;
        m_vertices = other.m_vertices;
        m_normals = other.m_normals;
        m_verticesCounts = other.m_verticesCounts;
        m_verticesIndices = other.m_verticesIndices;
        m_faceToVerts = other.m_faceToVerts;
        m_edgeToVerts = other.m_edgeToVerts;
		m_connected = other.m_connected;
        m_fnMesh.setObject(m_dagPath);
		m_matrcesC.resize(m_vertices.length());
        m_matrcesC = other.m_matrcesC;
        

    }
    return *this;
}


MFloatVectorArray MeshHandler::getMeshNormals()
{
    MFloatVectorArray normals;
    MStatus status = m_fnMesh.getVertexNormals(false, normals, MSpace::kObject);
    if (!status) {
        MGlobal::displayError("Failed to get mesh normals.");
    }
	return normals;
}

bool MeshHandler::intesectMesh(MPoint point, MVector rayDir)
{


    MFloatVector directions[] = {
    MFloatVector(1, 0, 0), MFloatVector(0, 1, 0), MFloatVector(0, 0, 1),
    MFloatVector(1, 1, 1).normal(), MFloatVector(-1, 1, 1).normal(),
    // ... add more ...
    };
    int numRays = 5;
    int insideVoteCount = 0;

    MFloatPointArray hitPoints;       // Stores the locations of all hits

    // We don't care about the order of hits, just the count
    bool sortHits = false;
    // Acceleration parameters (can be left null)
    MMeshIsectAccelParams accelParams = m_fnMesh.autoUniformGridParams();

    MStatus status;
    MIntArray* faceIds;
    // Default tolerance is fine
    float tolerance = 1e-6f;

    MFloatPoint hitPoint;
    float hitRayParam = 0.0f;
    int hitFace = -1;
    int hitTriangle = -1;
    float hitBary1 = 0.0f, hitBary2 = 0.0f;
    bool testBothDirections = false;
    float maxParam = 1e6f;
    for (int j = 0; j < 5;j++)
    {
		rayDir = directions[j];
        bool hit = m_fnMesh.closestIntersection(point,
            rayDir,
            nullptr,              // faceIds (nullptr => all)
            nullptr,              // triIds
            false,                // idsSorted
            MSpace::kWorld,
            maxParam,
            testBothDirections,
            &accelParams,
            hitPoint,
            &hitRayParam,
            &hitFace,
            &hitTriangle,
            &hitBary1,
            &hitBary2,
            status);
        if (!status)
        {
            MGlobal::displayWarning("Intersection test failed for point ");

        }

        if (hitPoints.length() % 2 == 1)
        {
            MGlobal::displayInfo("Intersected ");
            return true;
        }
    }

	return false;
}

std::set<int>  MeshHandler::findIndicesWithValue(int value)
{
    std::set<int>  indices;
    for (const auto& [key, pairVal] : m_edgeToVerts)
    {
        if (pairVal.first == value || pairVal.second == value)
        {
            indices.insert(key);
        }
    }
    return indices;
}

MPointArray MeshHandler::getTrianglePoints(int faceIndex)
{
    MPointArray trianglePoints;
    MIntArray vertices;
    m_fnMesh.getPolygonVertices(faceIndex, vertices);
    for (unsigned int i : vertices){
		MPoint point = m_vertices[i];
        trianglePoints.append(point);
    }

	return trianglePoints;
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


const std::unordered_map<int, MIntArray>& MeshHandler::getFacesIndices() const
{
    return m_faceToVerts;
}

const std::unordered_map<int, std::pair<int, int>>& MeshHandler::getEdgesIndices() const
{
    return m_edgeToVerts;
}

MPoint MeshHandler::getPoint(int index)
{
    return m_vertices[index];
}


void MeshHandler::setPoint(int index, MPoint newPoint)
{
    m_vertices[index] = newPoint;
}

MMatrix MeshHandler::getMatrixC(int vertexIdx)
{
    return m_matrcesC[vertexIdx];
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
    msg += " | Faces: "; msg += (int)m_faceToVerts.size();
    msg += " | Edges: "; msg += (int)m_edgeToVerts.size();
    MGlobal::displayInfo(msg);
}


void MeshHandler::updateMesh() {
    m_fnMesh.setPoints(m_vertices, MSpace::kObject);
    //m_fnMesh.setNormals(m_normals, MSpace::kObject);
    m_fnMesh.updateSurface();
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


