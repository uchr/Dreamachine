#pragma once

#include <vector>
#include <string>

struct Vector2 {
    float x, y;
};

struct Vector3 {
    float x, y, z;
};

struct Quaternion
{
    float x, y, z, w;
};

struct Mesh {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<int> indices;
    std::vector<std::vector<std::string>> texturePathStages;
};
