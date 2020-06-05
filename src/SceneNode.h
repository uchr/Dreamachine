#pragma once

#include "Mesh.h"

#include <Magnum/Magnum.h>
#include <Magnum/Math/Matrix4.h>

#include <optional>

namespace parser
{

struct Transofrmation {
    Vector3 translation;
    Vector3 rotation;
    float scale;
};

struct SceneNode
{
    std::string name;

    std::optional<Mesh> mesh;

    Vector3 position;
    Quaternion rotation;
    float scale;

    std::vector<SceneNode> children;

    Magnum::Matrix4 computeTransformationMatrix() const;
    Transofrmation computeTransformation() const;

    size_t numberOfMeshes() const;
    void print() const;
};

}