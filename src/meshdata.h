#pragma once
#include "mesh.h"
#include "collison.h"

class MeshData : public MUserData {
public:
    MeshData(
        float _factor,
        const std::vector<MPoint>& _deltas,
        const std::unordered_map<int, std::pair<int, int>>& _edgesIDX,
        const std::unordered_map<int, MIntArray>& _faceesIDX,
        const std::vector<MMatrix>& _m_matrcesC,
        const CollisonData& _collisonData,
        const MPointArray& _m_vertices,
        const MPointArray& _m_smooth
    )
        : MUserData(),
        factor(_factor),
        deltas(_deltas),
        edgesIDX(_edgesIDX),
        faceesIDX(_faceesIDX),
        m_matrcesC(_m_matrcesC),
        collisonData(_collisonData),
        m_vertices(_m_vertices),
        m_smooth(_m_smooth)
    {}

    float factor;

	std::vector<MPoint> deltas;

	std::unordered_map<int, std::pair<int, int>> edgesIDX;

	std::unordered_map<int, MIntArray> faceesIDX;

    std::vector<MMatrix> m_matrcesC;

	CollisonData collisonData;

    MPointArray m_vertices;

    MPointArray m_smooth;

    ~MeshData() = default;
};