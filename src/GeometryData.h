#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <optional>

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

struct MeshPart {
    std::pair<int, int> indexInterval;
    std::pair<int, int> vertexInterval;
    std::vector<std::filesystem::path> textures;
    std::optional<std::filesystem::path> alphaTexture;
};

struct Mesh
{
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<int> indices; // TODO: uint16_t
    std::vector<MeshPart> meshParts;
    bool smoothness = false;
};

}
