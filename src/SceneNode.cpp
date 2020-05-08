#include "SceneNode.h"

#include <Magnum/Math/Quaternion.h>

#include <iostream>

namespace
{

void numberOfMeshes(const parser::SceneNode& node, size_t& number)
{
    if (node.mesh.has_value())
        ++number;

    if (!node.children.empty()) {
        for (size_t i = 0; i < node.children.size(); ++i)
            numberOfMeshes(node.children[i], number);
    }
}

void print(const parser::SceneNode& node, std::string offset)
{
    std::cout << offset << node.name;
    if (node.mesh.has_value())
        std::cout << "(d)";
    std::cout << std::endl;

    if (!node.children.empty()) {
        for (size_t i = 0; i < node.children.size(); ++i)
            print(node.children[i], offset + " ");
    }
}

}

namespace parser
{

Magnum::Matrix4 SceneNode::computeTransformation() const {
    float magnumScale = 1.0f / scale;
    Magnum::Vector3 magnumPosition(position.x, position.y, position.z);
    Magnum::Quaternion magnumRotation(Magnum::Vector3(rotation.x, rotation.y, rotation.z), rotation.w);
    magnumPosition = magnumRotation.inverted().transformVector(magnumPosition);
    if (!magnumRotation.isNormalized()) {
        magnumScale *= magnumRotation.dot();
        magnumRotation = magnumRotation.normalized();
    }

    Magnum::Matrix4 rotationMatrix = Magnum::Matrix4::from(magnumRotation.toMatrix(), {});
    Magnum::Matrix4 scalingMatrix = Magnum::Matrix4::scaling(Magnum::Vector3(magnumScale));
    Magnum::Matrix4 translationMatrix = Magnum::Matrix4::translation(magnumPosition);
    Magnum::Matrix4 transformationMatrix = scalingMatrix * rotationMatrix * translationMatrix;

    return transformationMatrix;
}

size_t SceneNode::numberOfMeshes() const {
    size_t number = 0;
    ::numberOfMeshes(*this, number);
    return number;
}

void SceneNode::print() const {
    ::print(*this, "");
}

}
