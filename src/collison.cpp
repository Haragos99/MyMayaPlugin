#include "collison.h"
#include "tight_inclusion/ccd.hpp"

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

bool Collison::collisondetec(MeshHandler& mesh, MeshHandler& smooth)
{

    smallestTio = 1;
    tois.clear();
    Eigen::Vector3f v_t0, v_t1;
    Eigen::Vector3f f0_t0, f1_t0, f2_t0;
    Eigen::Vector3f f0_t1, f1_t1, f2_t1;
    float outtolerance;
    float toi = 0;
    bool isanycollied = false;


    int vindex = -1;
    int findex = -1;
    int eindex = -1;
    int eindex2 = -1;

    int pointsCount = mesh.getVertices().length();
    auto& faceIndices = mesh.getFacesIndices();
    auto& edgeIndices = mesh.getEdgesIndices();
    
    for (int vertexIdx = 0;  vertexIdx < pointsCount; ++vertexIdx)
    {
        if (deltas[vertexIdx].isCollied)
        {
            continue;
        }
        v_t1 = toEigenVec(mesh.getPoint(vertexIdx));
        v_t0 = toEigenVec(smooth.getPoint(vertexIdx));
        for (auto& face : faceIndices)
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
                tois.push_back(toi);
                if (toi < smallestTio)
                {
                    smallestTio = toi;
                    vindex = vertexIdx;
                    findex = faceIdx;
                }
                isanycollied = true;

            }
        }
    }

    
    for (auto edge : edgeIndices)
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
        for (auto edge2 : edgeIndices)
        {
            auto edgesidx2 = edge2.second;

            if (edgesidx.first == edgesidx2.first || edgesidx.second == edgesidx2.second || edgesidx.second == edgesidx2.first || edgesidx.first == edgesidx2.second)
            {
                continue;
            }

            // Convert OpenMesh vertices to Eigen vectors for t1
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

                if (toi < smallestTio)
                {
                    smallestTio = toi;
                    eindex = edge.first;
                    eindex2 = edge2.first;
                }
                isanycollied = true;


            }
        }
    }
    
    
    alfa = smallestTio;
    prevTio = smallestTio;
    setSmalest(vindex, findex, eindex, eindex2, mesh);
    setRestToi(alfa);
    for (int vertexIdx = 0; vertexIdx < pointsCount; ++vertexIdx)
    {
        setMeshTio(vertexIdx, mesh);
    }
	std::string alfastr = "Alfa: " + std::to_string(alfa);
    MGlobal::displayInfo(alfastr.c_str());
    MGlobal::displayInfo(std::to_string(pointsCount).c_str());


    return isanycollied;
}



void Collison::setRestToi(float newtoi)
{
    for (auto& d : deltas)
    {
        if (!d.isCollied)
        {
            d.toi = newtoi;
        }
    }
}

void Collison::setMeshTio(int vertexIdx, MeshHandler& mesh)
{
    MPoint newPoint = deltas[vertexIdx].getDeltaPoint(std::move(mesh.getMatrixC(vertexIdx)));
    mesh.setPoint(vertexIdx, newPoint);

}

void Collison::setSmalest(int vertexIdx, int f, int edegs, int edegs2,MeshHandler& mesh)
{
    if (edegs != -1)
    {

        auto& edgePoints = mesh.getEdgesIndices().at(edegs);
        deltas[edgePoints.first].toi = alfa;
        deltas[edgePoints.first].isCollied = true;
        setMeshTio(edgePoints.first, mesh);
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
	}


}

void Collison::restCollied()
{

}