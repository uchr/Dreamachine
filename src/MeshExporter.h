#pragma once

#include "Scene.h"

inline void exportSTL(const Mesh& mesh, std::ostream& out) {
    out << "solid STLExport" << std::endl;

    for (int i = 0; i < mesh.indices.size() / 3; ++i) {
        int t0 = mesh.indices[i * 3 + 0];
        int t1 = mesh.indices[i * 3 + 1];
        int t2 = mesh.indices[i * 3 + 2];
        Vector3 normal = mesh.normals[t0];
        out << "facet normal " << normal.x << " " << normal.y << " " << normal.z << std::endl;
        out << " outer loop" << std::endl;
        out << " vertex " << mesh.vertices[t0].x << " " << mesh.vertices[t0].y << " " << mesh.vertices[t0].z << std::endl;
        out << " vertex " << mesh.vertices[t1].x << " " << mesh.vertices[t1].y << " " << mesh.vertices[t1].z << std::endl;
        out << " vertex " << mesh.vertices[t2].x << " " << mesh.vertices[t2].y << " " << mesh.vertices[t2].z << std::endl;
        out << " endloop" << std::endl;
        out << "endfacet" << std::endl;
    }
    out << "endsolid STLExport" << std::endl;
}
