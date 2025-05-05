#include "pch.h"
#include "plugin.h"

MStatus MyPluginCmd::createCube()
{
    // When you are using pre-compiled headers, this source file is necessary for compilation to succeed.
    MStatus status;

    // Define the 8 vertices of the cube
    MPointArray vertices;
    vertices.append(MPoint(-1.0, -1.0, -1.0)); // Vertex 0
    vertices.append(MPoint(1.0, -1.0, -1.0)); // Vertex 1
    vertices.append(MPoint(1.0, -1.0, 1.0)); // Vertex 2
    vertices.append(MPoint(-1.0, -1.0, 1.0)); // Vertex 3
    vertices.append(MPoint(-1.0, 1.0, -1.0)); // Vertex 4
    vertices.append(MPoint(1.0, 1.0, -1.0)); // Vertex 5
    vertices.append(MPoint(1.0, 1.0, 1.0)); // Vertex 6
    vertices.append(MPoint(-1.0, 1.0, 1.0)); // Vertex 7

    // Define the number of vertices per face (6 faces, each with 4 vertices)
    MIntArray polygonCounts;
    for (int i = 0; i < 6; ++i) {
        polygonCounts.append(4);
    }

    // Define the vertex indices for each face
    MIntArray polygonConnects;
    // Bottom face
    polygonConnects.append(0); polygonConnects.append(1);
    polygonConnects.append(2); polygonConnects.append(3);
    // Top face
    polygonConnects.append(4); polygonConnects.append(5);
    polygonConnects.append(6); polygonConnects.append(7);
    // Front face
    polygonConnects.append(3); polygonConnects.append(2);
    polygonConnects.append(6); polygonConnects.append(7);
    // Back face
    polygonConnects.append(0); polygonConnects.append(1);
    polygonConnects.append(5); polygonConnects.append(4);
    // Left face
    polygonConnects.append(0); polygonConnects.append(3);
    polygonConnects.append(7); polygonConnects.append(4);
    // Right face
    polygonConnects.append(1); polygonConnects.append(2);
    polygonConnects.append(6); polygonConnects.append(5);

    // Create the mesh
    MFnMesh meshFn;
    MObject mesh = meshFn.create(
        vertices.length(),            // Number of vertices
        polygonCounts.length(),       // Number of polygons
        vertices,                     // Vertex positions
        polygonCounts,                // Number of vertices per polygon
        polygonConnects,              // Vertex indices
        MObject::kNullObj,            // Parent or owner (optional)
        &status
    );

    if (status != MStatus::kSuccess) {
        MGlobal::displayError("Failed to create the cube mesh.");
        return status;
    }

    MGlobal::displayInfo("A Cube the mesh created successfully.");

}