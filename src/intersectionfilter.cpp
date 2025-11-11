#include "intersectionfilter.h"




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
	// Implementation of intersection calculation between original and smooth meshes;

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
std::set<int> IntersectionFilter::clalculateIntersections(MeshHandler& original, MeshHandler& smooth)
{
	original.updateMesh();
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
		bool hit = fnMesh1.allIntersections(raySource, rayDir, NULL, NULL, false, MSpace::kWorld, 99999, false, &accelParams, false,
			hitPoints, &hitRayParams, &hitFaces, &hitTriangles, &hitBary1, &hitBary2, 0.000001f);

		if (hit)
		{
			fnMesh1.getClosestPointAndNormal(pos, closePoint, closeNormal, MSpace::kWorld, NULL, NULL);
			auto delta = pos - closePoint;
			//collision check with dot product
			auto angle = delta * closeNormal;
			if (angle < 0)
			{
				//MGlobal::displayInfo("Collision");
				collisionPoints.insert(idx);
			}
		}
	}
	return collisionPoints;
}



