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

}

bool Collison::collisondetec(MeshHandler& mesh, MeshHandler& smooth)
{
    return true;// TODO implemet it
}



void Collison::setRestToi(float newtoi)
{

}

void Collison::setMeshTio(int v, MeshHandler mesh)
{

}

void Collison::setSmalest(int v, int f, std::pair<int, int> edegs, MeshHandler mesh)
{

}

void Collison::restCollied()
{

}