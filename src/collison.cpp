#include "collison.h"
#include "tight_inclusion/ccd.hpp"

Collison::Collison() {
    err = Eigen::Vector3f(-1, -1, -1);  // Error bounds
    tmax = 1.0;
    tmaxiter = 1e7;
    tolerance = 1e-3;
    mc = 1e-6;
    smallestTio = 1.0f;
    alfa = 0;
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


    int vindex;
    int findex;


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
                //mesh.data(v).color = Vec(1, 0, 0);
                //deltas[v.idx()].toi = toi;
                tois.push_back(toi);
                //verteces.insert(v);
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

void Collison::setSmalest(int v, int f, std::pair<int, int> edegs, MeshHandler mesh)
{

}

void Collison::restCollied()
{

}