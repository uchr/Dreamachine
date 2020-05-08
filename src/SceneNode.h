#pragma once

#include "GeometryData.h"

#include <Magnum/Magnum.h>
#include <Magnum/Math/Matrix4.h>

#include <optional>

namespace parser
{

struct SceneNode
{
    std::string name;

    std::optional<Mesh> mesh;

    Vector3 position;
    Quaternion rotation;
    float scale;

    std::vector<SceneNode> children;

    Magnum::Matrix4 computeTransformation() const;

    size_t numberOfMeshes() const;
    void print() const;
};

}