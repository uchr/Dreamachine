#pragma once

#include <vector>
#include <string>

namespace parser
{

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

struct Mesh
{
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<int> indices; // TODO: uint16_t
    std::vector<std::vector<std::string>> texturePathStages;
};

}
