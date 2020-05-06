#pragma once

#include <optional>

#include "GeometryData.h"

namespace parser
{

struct SceneNode
{
    std::optional<Mesh> mesh;
    Vector3 position;
    Quaternion rotation;
    float scale;

    std::vector<SceneNode> children;
};

}