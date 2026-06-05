#include "collison.h"
#include "tight_inclusion/ccd.hpp"
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>
#include <tbb/concurrent_vector.h>
#include <limits>

Collison::Collison(std::vector<MPoint> v) {
    err = Eigen::Vector3f(-1, -1, -1);  // Error bounds
    tmax = 1.0;
    tmaxiter = 1e7;
    tolerance = 1e-3;
    mc = 1e-6;
    smallestTio = 1.0f;
    alfa = 0;
    init(v);
}

void Collison::init(std::vector<MPoint> v)
{
    if (deltas.empty())
    {
        for (auto& d : v)
        {
            deltas.push_back(Delta(d, 1.0f, false));
        }

    }
}

bool Collison::collisondetec(MeshHandler& mesh, MeshHandler& smooth, CollisonData& data)
{
    smallestTio = 1;
    tois.clear();
    Eigen::Vector3f v_t0, v_t1;
    Eigen::Vector3f f0_t0, f1_t0, f2_t0;
    Eigen::Vector3f f0_t1, f1_t1, f2_t1;
    float outtolerance;
    float toi = 0;
    bool isanycollied = false;

    std::set<int> veIDX;
    std::unordered_map<int, std::pair<int, int>> edIDX;
    std::unordered_map<int, MIntArray> faIDX;

    int vindex = -1;
    int findex = -1;
    int eindex = -1;
    int eindex2 = -1;

    int pointsCount = mesh.getVertices().length();
    auto& faceIndices = mesh.getNearbyFaces();
    //auto& edgeIndices = mesh.getEdgesIndices();
    MGlobal::displayInfo("Start VF");


    MString sizeinfo = "Face Size: "; sizeinfo +=(int)facesIDX.size();
	sizeinfo += " Edge Size: "; sizeinfo += (int)edgesIDX.size();
	sizeinfo += " Vertex Size: "; sizeinfo += (int)vertexesIDX.size();
	MGlobal::displayInfo(sizeinfo);
    MGlobal::displayInfo("Start VF");

    for (int vertexIdx : vertexesIDX)
    {
        if (deltas[vertexIdx].isCollied)
        {
            continue;
        }
        v_t1 = toEigenVec(mesh.getPoint(vertexIdx));
        v_t0 = toEigenVec(smooth.getPoint(vertexIdx));
        for (auto& face : facesIDX)
        {
            /// it Must be triangle
            int faceIdx = face.first;
            MIntArray faceVertecesIdx = face.second;

            int facevercesIdx0 = faceVertecesIdx[0];
            int facevercesIdx1 = faceVertecesIdx[1];
            int facevercesIdx2 = faceVertecesIdx[2];

            bool isInTriangle = facevercesIdx0 == vertexIdx || facevercesIdx1 == vertexIdx || facevercesIdx2 == vertexIdx;
            if (isInTriangle)
            {
                continue;
            }

            f0_t1 = toEigenVec(mesh.getPoint(facevercesIdx0));
            f0_t0 = toEigenVec(smooth.getPoint(facevercesIdx0));

            f1_t1 = toEigenVec(mesh.getPoint(facevercesIdx1));
            f1_t0 = toEigenVec(smooth.getPoint(facevercesIdx1));

            f2_t1 = toEigenVec(mesh.getPoint(facevercesIdx2));
            f2_t0 = toEigenVec(smooth.getPoint(facevercesIdx2));

            bool iscollied = ticcd::vertexFaceCCD(
                v_t0, f0_t0, f1_t0, f2_t0,
                v_t1, f0_t1, f1_t1, f2_t1,
                err, mc, toi, tolerance, tmax, tmaxiter, outtolerance
            );
            if (iscollied) 
            {
                vertexes.insert(vertexIdx);
                for (int i : faceVertecesIdx)
                {
                    vertexes.insert(i);
                }
                tois.push_back(toi);
                if (toi < smallestTio)
                {
                    smallestTio = toi;
                    vindex = vertexIdx;
                    findex = faceIdx;
                }
                faIDX[faceIdx] = faceVertecesIdx;
                veIDX.insert(vertexIdx);
                isanycollied = true;

            }
        }
    }

    MGlobal::displayInfo("Start EE");
    for (auto edge : edgesIDX)
    {

		auto edgesidx = edge.second;
        if (deltas[edgesidx.first].isCollied || deltas[edgesidx.second].isCollied)
        {
            continue;
        }

        // Convert OpenMesh vertices to Eigen vectors for t1
        Eigen::Vector3f ea0_t1 = toEigenVec(mesh.getPoint(edgesidx.first));
        Eigen::Vector3f ea1_t1 = toEigenVec(mesh.getPoint(edgesidx.second));

        // Assume edge 2 moves (displacement example for t0)
        Eigen::Vector3f ea0_t0 = toEigenVec(smooth.getPoint(edgesidx.first));
        Eigen::Vector3f ea1_t0 = toEigenVec(smooth.getPoint(edgesidx.second));
        for (auto edge2 : edgesIDX)
        {
            auto edgesidx2 = edge2.second;

            if (edgesidx.first == edgesidx2.first || edgesidx.second == edgesidx2.second || edgesidx.second == edgesidx2.first || edgesidx.first == edgesidx2.second)
            {
                continue;
            }

            // Convert vertices to Eigen vectors for t1
            Eigen::Vector3f eb0_t1 = toEigenVec(mesh.getPoint(edgesidx2.first));
            Eigen::Vector3f eb1_t1 = toEigenVec(mesh.getPoint(edgesidx2.second));

            // Assume edge 2 moves (displacement example for t0)
            Eigen::Vector3f eb0_t0 = toEigenVec(smooth.getPoint(edgesidx2.first));
            Eigen::Vector3f eb1_t0 = toEigenVec(smooth.getPoint(edgesidx2.second));
            // Perform edge-edge collision detection
            bool is_colliding = ticcd::edgeEdgeCCD(
                ea0_t0, ea1_t0, eb0_t0, eb1_t0,  // Edges at time t0
                ea0_t1, ea1_t1, eb0_t1, eb1_t1,  // Edges at time t1
                err,                             // Error bounds
                mc,                              // Minimum separation
                toi,                             // Time of impact (output)
                tolerance,                       // Solving precision
                tmax,                           // Time interval upper bound (0 <= t_max <= 1)
                tmaxiter,                         // Maximum iterations
                outtolerance,                // Output precision under max_itr
                true                             // Refine for zero toi
            );

            if (is_colliding) {
                vertexes.insert(edgesidx2.first);
                vertexes.insert(edgesidx2.second);

                vertexes.insert(edgesidx.first);
                vertexes.insert(edgesidx.second);
                if (toi < smallestTio)
                {
                    smallestTio = toi;
                    eindex = edge.first;
                    eindex2 = edge2.first;
                }
                edIDX[edge.first] = edge.second;
                edIDX[edge2.first] = edge2.second;
                isanycollied = true;


            }
        }
    }
    
    alfa = smallestTio;
    prevTio = smallestTio;
    setSmalest(vindex, findex, eindex, eindex2, mesh, data);
    setRestToi(alfa);
    for (int vertexIdx = 0; vertexIdx < pointsCount; ++vertexIdx)
    {
        setMeshTio(vertexIdx, mesh);
    }
    if (veIDX.size() > 0)
    {
		vertexesIDX = veIDX;
    }
    if (faIDX.size() > 0)
    {
		facesIDX = faIDX;
    }
    if (edIDX.size() > 0)
    {
		edgesIDX = edIDX;
    }

	std::string alfastr = "Alfa: " + std::to_string(alfa);
    MGlobal::displayInfo(alfastr.c_str());
    MGlobal::displayInfo(std::to_string(pointsCount).c_str());

    return isanycollied;
}






bool Collison::collisondetecPA(MeshHandler& mesh,
    MeshHandler& smooth,
    CollisonData& data)
{

    tois.clear();
    smallestTio = 1.0f;
    prevTio = alfa;

    const int pointsCount = mesh.getVertices().length();

    tbb::concurrent_vector<TOIRec> collisions;

    // Snapshot index lists (never iterate shared containers directly)
    std::vector<int> verts(vertexesIDX.begin(), vertexesIDX.end());

    std::vector<int> faceIds;
    std::vector<MIntArray> faceVerts;
    for (auto& f : facesIDX) 
    {
        faceIds.push_back(f.first);
        faceVerts.push_back(f.second);
    }

    std::vector<int> edgeIds;
    std::vector<std::pair<int, int>> edgeVerts;
    for (auto& e : edgesIDX) 
    {
        edgeIds.push_back(e.first);
        edgeVerts.push_back(e.second);
    }

    tbb::parallel_for(
        tbb::blocked_range2d<size_t>(0, verts.size(), 0, faceIds.size()),
        [&](const tbb::blocked_range2d<size_t>& r)
        {
            for (size_t vi = r.rows().begin(); vi != r.rows().end(); ++vi) 
            {
                int vIdx = verts[vi];
                if (deltas[vIdx].isCollied)
                {
                    continue;
                } 

                Eigen::Vector3f v0 = toEigenVec(smooth.getPoint(vIdx));
                Eigen::Vector3f v1 = toEigenVec(mesh.getPoint(vIdx));

                for (size_t fi = r.cols().begin(); fi != r.cols().end(); ++fi) 
                {
                    const auto& fvs = faceVerts[fi];
					MBoundingBox faceBox;
                    if (fvs[0] == vIdx || fvs[1] == vIdx || fvs[2] == vIdx) 
                    {
                        continue; 
                    }

					faceBox.expand(mesh.getPoint(fvs[0]));
					faceBox.expand(mesh.getPoint(fvs[1]));
					faceBox.expand(mesh.getPoint(fvs[2]));

                    if(!faceBox.contains((mesh.getPoint(vIdx))))
                    {
                        continue;
					}

                    Eigen::Vector3f f0_0 = toEigenVec(smooth.getPoint(fvs[0]));
                    Eigen::Vector3f f1_0 = toEigenVec(smooth.getPoint(fvs[1]));
                    Eigen::Vector3f f2_0 = toEigenVec(smooth.getPoint(fvs[2]));

                    Eigen::Vector3f f0_1 = toEigenVec(mesh.getPoint(fvs[0]));
                    Eigen::Vector3f f1_1 = toEigenVec(mesh.getPoint(fvs[1]));
                    Eigen::Vector3f f2_1 = toEigenVec(mesh.getPoint(fvs[2]));

                    float toi = 0.f;
                    bool hit = ticcd::vertexFaceCCD(
                        v0, f0_0, f1_0, f2_0,
                        v1, f0_1, f1_1, f2_1,
                        err, mc, toi, tolerance, tmax, tmaxiter, tolerance
                    );

                    if (hit) 
                    {
                        collisions.push_back({ toi, CCDType::VF, vIdx, faceIds[fi] });
                    }
                }
            }
        });


    tbb::parallel_for(
        tbb::blocked_range2d<size_t>(0, edgeIds.size(), 0, edgeIds.size()),
        [&](const tbb::blocked_range2d<size_t>& r)
        {
            for (size_t i = r.rows().begin(); i != r.rows().end(); ++i) 
            {
                const auto& eA = edgeVerts[i];
                if (deltas[eA.first].isCollied || deltas[eA.second].isCollied) 
                { 
                    continue; 
                }

                Eigen::Vector3f a0_0 = toEigenVec(smooth.getPoint(eA.first));
                Eigen::Vector3f a1_0 = toEigenVec(smooth.getPoint(eA.second));
                Eigen::Vector3f a0_1 = toEigenVec(mesh.getPoint(eA.first));
                Eigen::Vector3f a1_1 = toEigenVec(mesh.getPoint(eA.second));
				MBoundingBox edge1;
				edge1.expand(mesh.getPoint(eA.first));
				edge1.expand(mesh.getPoint(eA.second));
                   

                for (size_t j = r.cols().begin(); j != r.cols().end(); ++j) 
                {
                    if (j <= i) 
                    { 
                        continue;
                    }
;
                    const auto& eB = edgeVerts[j];
                    if (eA.first == eB.first || eA.first == eB.second ||
                        eA.second == eB.first || eA.second == eB.second)
                    {
                        continue;
                    }

                    MBoundingBox edge2;
                    edge2.expand(mesh.getPoint(eB.first));
                    edge2.expand(mesh.getPoint(eB.second));

                    if(!edge1.intersects(edge2))
                    {
                         continue;
					}

                    Eigen::Vector3f b0_0 = toEigenVec(smooth.getPoint(eB.first));
                    Eigen::Vector3f b1_0 = toEigenVec(smooth.getPoint(eB.second));
                    Eigen::Vector3f b0_1 = toEigenVec(mesh.getPoint(eB.first));
                    Eigen::Vector3f b1_1 = toEigenVec(mesh.getPoint(eB.second));

                    float toi = 0.f;
                    bool hit = ticcd::edgeEdgeCCD(
                        a0_0, a1_0, b0_0, b1_0,
                        a0_1, a1_1, b0_1, b1_1,
                        err, mc, toi, tolerance, tmax, tmaxiter, tolerance, true
                    );

                    if (hit) 
                    {
                        collisions.push_back({ toi, CCDType::EE, -1,-1,
                                               edgeIds[i], edgeIds[j] });
                    }
                }
            }
        });


    int vIdx = -1, fIdx = -1, e1 = -1, e2 = -1;

    for (auto& c : collisions) 
    {
        tois.push_back(c.toi);
        if (c.toi < smallestTio) 
        {
            smallestTio = c.toi;
            vIdx = c.v; fIdx = c.f;
            e1 = c.e1; e2 = c.e2;
        }
    }

    alfa = smallestTio;
    prevTio = smallestTio;


    std::set<int> nextVerts;
    std::unordered_map<int, MIntArray> nextFaces;
    std::unordered_map<int, std::pair<int, int>> nextEdges;

    for (auto& c : collisions) 
    {
        if (c.type == CCDType::VF) 
        {
            nextVerts.insert(c.v);
            auto& fv = facesIDX[c.f];
            nextFaces[c.f] = fv;
            for (int vi : fv) 
            { 
                vertexes.insert(vi);
            }
            data.collidedFacesIdx.insert(c.f);
            data.collidedVertecesIdx.insert(c.v);
			vertexes.insert(c.v);

        }
        else 
        {
            nextEdges[c.e1] = edgesIDX[c.e1];
            nextEdges[c.e2] = edgesIDX[c.e2];

            auto& a = edgesIDX[c.e1];
            auto& b = edgesIDX[c.e2];
            nextVerts.insert(a.first); nextVerts.insert(a.second);
            nextVerts.insert(b.first); nextVerts.insert(b.second);
            data.collidedEdgesIdx.insert(c.e1);
            data.collidedEdgesIdx.insert(c.e2);
			vertexes.insert(a.first); vertexes.insert(a.second);
        }
    }

    setSmalest(vIdx, fIdx, e1, e2, mesh, data);
    setRestToi(alfa);

    for (int i = 0; i < pointsCount; ++i)
    {
        setMeshTio(i, mesh);
    }


    vertexesIDX = std::move(nextVerts);
    facesIDX = std::move(nextFaces);
    edgesIDX = std::move(nextEdges);

    std::string alfastr = "Alfa: " + std::to_string(alfa);
    MGlobal::displayInfo(alfastr.c_str());

	return !collisions.empty(); // Return true if any collision was detected
}


void Collison::setRestToi(float newtoi)
{
    for (auto& delta : deltas)
    {
        if (!delta.isCollied)
        {
            delta.toi = newtoi;
        }
    }
}

void Collison::setMeshTio(int vertexIdx, MeshHandler& mesh)
{
    MPoint newPoint = deltas[vertexIdx].getDeltaPoint(std::move(mesh.getMatrixC(vertexIdx)));
    mesh.setPoint(vertexIdx, newPoint);
}

void Collison::setSmalest(int vertexIdx, int f, int edegs, int edegs2,MeshHandler& mesh, CollisonData& data)
{
	std::string msg = "Set smalest toi " + std::to_string(vertexIdx) + " " + std::to_string(f) + " " + std::to_string(edegs) + " " + std::to_string(edegs2);
	MGlobal::displayInfo(msg.c_str());
    if (edegs != -1 || edegs2 != -1)
    {
        auto& edgePoints = mesh.getEdgesIndices().at(edegs);

		// Set TOI for the vertices of the colliding edge
        deltas[edgePoints.first].toi = alfa;
        deltas[edgePoints.first].isCollied = true;
        setMeshTio(edgePoints.first, mesh);

		// Set TOI for the second vertex of the colliding edge
        deltas[edgePoints.second].toi = alfa;
        deltas[edgePoints.second].isCollied = true;
        setMeshTio(edgePoints.second, mesh);

        auto& edgePoints2 = mesh.getEdgesIndices().at(edegs2);

        deltas[edgePoints2.first].toi = alfa;
        deltas[edgePoints2.first].isCollied = true;
        setMeshTio(edgePoints2.first, mesh);

        deltas[edgePoints2.second].toi = alfa;
        deltas[edgePoints2.second].isCollied = true;
        setMeshTio(edgePoints2.second, mesh);

		data.collidedEdgesIdx.insert(edegs);
		data.collidedEdgesIdx.insert(edegs2);
    }
    else
    {
        if (f == -1 || vertexIdx == -1)
        {
            return;
		}
        deltas[vertexIdx].toi = alfa;
        deltas[vertexIdx].isCollied = true;
        setMeshTio(vertexIdx, mesh);
        auto& facePoints = mesh.getFacesIndices().at(f);
        for (int pointIdx : facePoints)
        {
            deltas[pointIdx].toi = alfa;
            deltas[pointIdx].isCollied = true;
            setMeshTio(pointIdx, mesh);
        }
		data.collidedVertecesIdx.insert(vertexIdx);
		data.collidedFacesIdx.insert(f);
	}
}

void Collison::restCollied()
{

}