#pragma once

#include "Geometry.h"

#include <vector>
#include <string>
#include <filesystem>
#include <optional>

namespace parser
{

struct MeshPart {
    std::pair<int, int> indexInterval;
    std::pair<int, int> vertexInterval;
    std::vector<std::filesystem::path> textures;
    std::optional<std::filesystem::path> alphaTexture;
};

struct Mesh
{
    std::string name;

    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<uint16_t> indices;

    std::vector<MeshPart> meshParts;

    bool smoothness = false;
};

}
