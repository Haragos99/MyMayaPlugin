#include "intersectionfilter.h"
#include <thread> 
#include <tbb/parallel_for.h>
#include <numeric>

IntersectionFilter::IntersectionFilter(MeshHandler& target)
	: m_intersector()
{
	target.updateMesh();
	// Initialize the intersector with the mesh object
	auto meshObj = target.getMeshObject();

	MStatus status;

	// Create an in-memory mesh data container (not visible in the scene)
	MFnMeshData meshDataFn;
	meshDataObj = meshDataFn.create(&status);
	MFnMesh meshFn;
	MObject smoothMeshObj = meshFn.copy(meshObj, meshDataObj, &status);
	auto dataPoints = target.getVertices();
	meshFn.setPoints(dataPoints, MSpace::kObject);

	status = m_intersector.create(smoothMeshObj);
	if (!status) {
		MGlobal::displayError("Failed to create MMeshIntersector.");
	}
	
}


void IntersectionFilter::clalculateIntersections(const MPointArray& original, const MPointArray& smooth, MeshHandler& target, MeshHandler& sm, CollisonData& data)
{
	MGlobal::displayInfo(MString("VertexX: ") );
	
	double epsilon = 1e-6;
	auto normlas = target.getNormals();
	auto& facesIndices = target.getFacesIndices();
	for (unsigned int i = 0; i < 0; ++i)
	{

		const MPoint& p0 = smooth[i];  // t0
		const MPoint& p1 = original[i];    // t1
		auto pon = p1;
		double motionDist = p0.distanceTo(p1);

		MPointOnMesh closest;
		auto f = m_intersector.getClosestPoint(p1, closest);
		if (f != MS::kSuccess)
		{
			continue;
		}
		auto closestPoint = closest.getPoint();
		int faceidx = closest.faceIndex();

		auto fa = facesIndices.at(faceidx);
		MBoundingBox box;
		std::set<int> edge;
		MBoundingBox box2;

		for(int vi : fa)
		{
			if(vi == i)
			{
				continue;
			}
			MPoint otherPoint = target.getPoint(vi);
			box.expand(otherPoint);

			auto ed = target.findIndicesWithValue(vi);
			edge.insert(ed.begin(), ed.end());

			
		}

		
		int connectedIdx = *target.getConnectedVertices(i).begin();
		MPoint connectedPoint = target.getPoint(connectedIdx);

		float scale = (connectedPoint - pon).length() / 1;

		//MGlobal::displayInfo(MString("Scale: ") + scale);

		MVector vectr = MVector(scale, scale, scale);
		pon += vectr;
		box2.expand(pon);
		pon = original[i];
		pon -= vectr;

		box2.expand(pon);


		double dist = p0.distanceTo(closest.getPoint());

		double relativeDist = motionDist-dist;
		//data.smoothmesh.push_back(box);
		//data.mesh.push_back(box2);
		if (box.intersects(box2))
		{
			auto ed = target.findIndicesWithValue(i);
			edge.insert(ed.begin(), ed.end());
			//data.intersected.push_back(box);
			//vertexIndices.insert(i);
			//faceIndices[faceidx] = fa;
			//edgeIndices.insert(edge.begin(), edge.end());
		}


		if (relativeDist > 0.1)
		{
			//int faceIdx= closest.faceIndex();
			int vertexIdx = i;
			//.push_back(vertexIdx);
			//faceIndices.push_back(faceIdx);
			//MGlobal::displayInfo(MString("Vertex IDX: ") + i);
		}
		MVector normal = normlas[i];


		//bool found = sm.intesectMesh(p1, -normal);

		//if(found)
		{
			//vertexIndices.push_back(i);
			//MGlobal::displayInfo(MString("Vertex IDX: ") + i);
		}




			
	}
	


	for (auto face : sm.getFacesIndices())
	{
		MBoundingBox box;
		box.expand(sm.getPoint(face.second[0]));
		box.expand(sm.getPoint(face.second[1]));
		box.expand(sm.getPoint(face.second[2]));
		double factor = 1;
		MPoint ctr = box.center();
		MVector half = (box.max() - ctr);
		half *= factor;
		box = MBoundingBox(ctr - half, ctr + half);


		data.smoothmesh.push_back(box);

	}

	float scale = 0.15;
	for (auto face2 : target.getVerticesIndices())
	{
		MVector vectr = MVector(scale, scale, scale);
		MBoundingBox box2;
		auto p = target.getPoint(face2);
		p += vectr;
		box2.expand(p);
		p = target.getPoint(face2);
		p -= vectr;
		box2.expand(p);

		data.mesh.push_back(box2);
	}

	int i = 0;
	for (auto b : data.mesh)
	{
		int j = 0;
		for (auto b2 : data.smoothmesh)
		{
			if (b2.intersects(b))
			{
				data.intersected.push_back(b);
				data.intersected.push_back(b2);
				auto& f = facesIndices.at(j);
				vertexIndices.insert(i);
				faceIndices[j] = f;

			}
			j++;
		}
		i++;
	}


	MGlobal::displayInfo(
		MString("Intersections calculated: ")
		+ MString() + std::to_string(data.intersected.size()).c_str()
		+ " Face: "
		+ std::to_string(faceIndices.size()).c_str()
	);
}

bool IntersectionFilter::isPointInPlane(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c)
{
	// Define a small tolerance value (epsilon)
	const double epsilon = 1e-9;
	MVector pa = p - a;
	MVector pb = p - b;
	MVector pc = p - c;
	return std::abs(mixed(pa, pb, pc)) <= epsilon;
}

double IntersectionFilter::mixed(MVector& a , MVector& b, MVector& c)
{
	auto cross = b ^ c;
	return a * cross;
}

bool IntersectionFilter::isPointInTriangle(const MPoint& p, const MPoint& a, const MPoint& b, const MPoint& c)
{
	if(!isPointInPlane(p, a, b, c))
	{
		return false;
	}

	MVector ba = b - a;
	MVector ca = c - a;
	MVector pa = p - a;

	const auto normDir = ba ^ ca;
	if((normDir * (ba ^ pa)) < 0)
	{
		return false;
	}

	MVector cb = c - b;
	MVector pb = p - b;
	if((normDir * (cb ^ pb)) < 0)
	{
		return false;
	}

	MVector pc = p - c;
	MVector ac = a - c;
	if((normDir * (ac ^ pc)) < 0)
	{
		return false;
	}
	return true;
}


bool IntersectionFilter::triangleIntersectsTriangle(const MPoint& v0, const MPoint& v1, const MPoint& v2,
	const MPoint& u0, const MPoint& u1, const MPoint& u2,
	double epsilon) {
	/*
	if(!(isPointInTriangle(v0, u0,  u1,  u2) ||
		isPointInTriangle(v1, u0,  u1,  u2) ||
		isPointInTriangle(v2, u0,  u1,  u2) ||
		isPointInTriangle(u0, v0,  v1,  v2) ||
		isPointInTriangle(u1, v0,  v1,  v2) ||
		isPointInTriangle(u2, v0,  v1,  v2)))
	{
		return false;
	}
	*/
	
	// Edge vectors
	MVector e1 = v1 - v0;
	MVector e2 = v2 - v0;
	MVector n1 = e1 ^ e2;  // Triangle 1 normal

	// Check if triangle 2 vertices are on same side of plane 1
	double d1 = n1 * (u0 - v0);
	double d2 = n1 * (u1 - v0);
	double d3 = n1 * (u2 - v0);

	if (fabs(d1) < epsilon) d1 = 0;
	if (fabs(d2) < epsilon) d2 = 0;
	if (fabs(d3) < epsilon) d3 = 0;

	// All on same side - no intersection
	if ((d1 > 0 && d2 > 0 && d3 > 0) || (d1 < 0 && d2 < 0 && d3 < 0)) {
		return false;
	}

	// Check if triangle 1 vertices are on same side of plane 2
	MVector f1 = u1 - u0;
	MVector f2 = u2 - u0;
	MVector n2 = f1 ^ f2;

	double d4 = n2 * (v0 - u0);
	double d5 = n2 * (v1 - u0);
	double d6 = n2 * (v2 - u0);

	if (fabs(d4) < epsilon) d4 = 0;
	if (fabs(d5) < epsilon) d5 = 0;
	if (fabs(d6) < epsilon) d6 = 0;

	if ((d4 > 0 && d5 > 0 && d6 > 0) || (d4 < 0 && d5 < 0 && d6 < 0)) {
		return false;
	}

	// Triangles are coplanar or intersecting
	// For coplanar case, need 2D overlap test
	if (fabs(d1) < epsilon && fabs(d2) < epsilon && fabs(d3) < epsilon) {
		// Coplanar - do 2D projection test (simplified here)
		return true;  // Conservative - report as intersection
	}

	return true;  // Triangles intersect
}


bool IntersectionFilter::areFacesAdjacent(std::shared_ptr<MItMeshPolygon> polyIter, int faceA, int faceB)
{

	// Set iterator to faceIndexA
	int prevIndex = -1;
	MStatus status = polyIter->setIndex(faceA, prevIndex);
	if (status != MS::kSuccess) {
		return false;
	}

	bool isAdj = polyIter->isConnectedToFace(faceB, &status);
	
	if (status != MS::kSuccess) {
		return false;
	}

	return isAdj;
}



// Also check if faces share vertices and are very close (nearly coplanar neighbors)
bool IntersectionFilter::areFacesNearbyNeighbors(const MFnMesh& meshFn, int faceA, int faceB, double threshold ) {
	MIntArray vertsA, vertsB;
	meshFn.getPolygonVertices(faceA, vertsA);
	meshFn.getPolygonVertices(faceB, vertsB);

	int sharedVerts = 0;
	for (unsigned int i = 0; i < vertsA.length(); i++) {
		for (unsigned int j = 0; j < vertsB.length(); j++) {
			if (vertsA[i] == vertsB[j]) {
				sharedVerts++;
			}
		}
	}

	// If they share 2+ vertices, check if coplanar
	if (sharedVerts >= 2) {
		MVector normalA, normalB;
		MItMeshPolygon polyA(meshFn.object());
		MItMeshPolygon polyB(meshFn.object());
		int prev;
		polyA.setIndex(faceA, prev);
		polyB.setIndex(faceB, prev);

		polyA.getNormal(normalA);
		polyB.getNormal(normalB);

		// Check if normals are similar (coplanar or nearly coplanar)
		double dot = normalA * normalB;
		if (dot > 0.99) { // ~8 degree tolerance
			return true;
		}
	}

	return false;
}




std::set<int> IntersectionFilter::clalculateIntersections(MeshHandler& original, MeshHandler& smooth)
{
	original.updateMesh();
	original.recalculateNormals();
	auto normlas = original.getNormals();
	auto& points = original.getVertices();
	//allIntersections flags
	MMeshIsectAccelParams accelParams;
	accelParams = original.getIntersectParameters();
	smooth.updateMesh();
	bool sortHits = false;
	float tolerance = 0.0001f;
	MFloatPointArray hitPoints;
	MFloatArray hitRayParams;
	MIntArray hitFaces;
	MIntArray hitTriangles;
	MFloatArray hitBary1;
	MFloatArray hitBary2;
	auto& fnMesh1 = smooth.getMeshFunction();

	MPoint closePoint;
	MVector closeNormal;
	std::set<int> collisionPoints;
	for (int idx = 0; idx < points.length(); ++idx) {
		// e.g. get position
		MPoint pos = points[idx];
		// do your work

		MFloatVector normal = -normlas[idx];
		auto raySource = MFloatPoint(pos[0], pos[1], pos[2], 1.0);

		auto rayDir = MFloatVector(normal[0], normal[1], normal[2]);
		bool hit = fnMesh1.allIntersections(raySource, rayDir, NULL, NULL, false, MSpace::kObject, 99999, false, &accelParams, false,
			hitPoints, &hitRayParams, &hitFaces, &hitTriangles, &hitBary1, &hitBary2, tolerance);

		if (hit)
		{
			fnMesh1.getClosestPointAndNormal(pos, closePoint, closeNormal, MSpace::kWorld, NULL, NULL);
			auto delta = pos - closePoint;
			//collision check with dot product
			auto angle = delta * closeNormal;
			if (angle < 0)
			{
				MGlobal::displayInfo("ACollision");
				collisionPoints.insert(idx);
			}
		}
	}
	MGlobal::displayInfo("Collision");
	return collisionPoints;
}







// It can make faster with BHV tree or spatial hash

std::set<int>  IntersectionFilter::findSelfCollidingTriangles(MeshHandler& target)
{
	target.updateMesh();
	double tolerance = 1e-6;
	double minAngle = 20.0;
	int numPolys = target.getFacesIndices().size();
	std::vector<MVector> faceNormals(numPolys);
	// Step 1: Build bounding boxes for all faces
	std::vector<MBoundingBox> faceBBoxes(numPolys);
	std::vector<MPoint> faceCenters(numPolys);

	auto polyIter = target.getPolygonIterator(nullptr);
	for (; !polyIter->isDone(); polyIter->next()) {
		int idx = polyIter->index();
		MPointArray points;
		polyIter->getPoints(points);

		MBoundingBox bbox;
		for (unsigned int i = 0; i < points.length(); i++) {
			bbox.expand(points[i]);
		}
		// Small padding to avoid numerical issues
		MPoint bmin = bbox.min();
		MPoint bmax = bbox.max();
		bbox.expand(MPoint(bmin.x - tolerance, bmin.y - tolerance, bmin.z - tolerance));
		bbox.expand(MPoint(bmax.x + tolerance, bmax.y + tolerance, bmax.z + tolerance));

		polyIter->getNormal(faceNormals[idx]);
		faceBBoxes[idx] = bbox;
		faceCenters[idx] = polyIter->center();
	}




	// Step 3: Check for intersections
	std::set<int> intersectingFaceSet;
	for (int i = 0; i < numPolys; i++) {
		std::vector<int> candidateFaces;


			// Check all faces if no spatial hash
		candidateFaces.resize(numPolys);
		std::iota(candidateFaces.begin(), candidateFaces.end(), 0);
		

		for (int j : candidateFaces) {
			
			if (j <= i) 
			{
				continue; // Avoid duplicate checks
			}  
			if (areFacesAdjacent(polyIter, i, j)) 
			{ 
				continue; 
			}

			if(areFacesNearbyNeighbors(target.getMeshFunction(), i, j))
			{
				continue;
			}

			// Bounding box test first
			if (!faceBBoxes[i].intersects(faceBBoxes[j]))
			{
				continue;
			}

			// Normal angle test - skip if faces point in very similar directions
			double normalDot = faceNormals[i] * faceNormals[j];
			double angleRad = acos(std::max(-1.0, std::min(1.0, normalDot)));
			double angleDeg = angleRad * 180.0 / M_PI;

			// Skip faces with similar orientations that are likely part of same surface
			if (angleDeg < minAngle) {
				continue;
			}

			// Distance test (quick rejection)
			int prev;
			polyIter->setIndex(i, prev);
			auto polyCenter = polyIter->center();

			polyIter->setIndex(j, prev);
			auto polyCenterNext = polyIter->center();

			double dist = polyCenter.distanceTo(polyCenterNext);
			double maxSize = std::max(
				faceBBoxes[i].max().distanceTo(faceBBoxes[i].min()),
				faceBBoxes[j].max().distanceTo(faceBBoxes[j].min())
			);

			if (dist > maxSize)
			{
				continue;
			}

			// Precise triangle-triangle intersection test

			auto triA = target.getTrianglePoints(i);
			auto triB = target.getTrianglePoints(j);
			
			bool intersects = false;
			
			if (triangleIntersectsTriangle(
				triA[0], triA[1], triA[2],
				triB[0], triB[1], triB[2],
				tolerance)) 
			{
				intersectingFaceSet.insert(i);
				intersectingFaceSet.insert(j);
			}
			
		}
	}

	MGlobal::displayInfo(
		MString("Self-intersecting faces found: ")
		+ MString() + std::to_string(intersectingFaceSet.size()).c_str()
	);
	return intersectingFaceSet;
}








