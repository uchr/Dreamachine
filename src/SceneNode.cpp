#include "SceneNode.h"

#include <Magnum/Math/Quaternion.h>

#include <fmt/format.h>

namespace
{

Magnum::Vector3 toEulerAngles(Magnum::Quaternion q) {
    Magnum::Vector3 angles;

    // roll (x-axis rotation)
    double sinr_cosp = 2 * (q.scalar() * q.vector().x() + q.vector().y() * q.vector().z());
    double cosr_cosp = 1 - 2 * (q.vector().x() * q.vector().x() + q.vector().y() * q.vector().y());
    angles.x() = std::atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    double sinp = 2 * (q.scalar() * q.vector().y() - q.vector().z() * q.vector().x());
    if (std::abs(sinp) >= 1)
        angles.y() = std::copysign(Magnum::Math::Constants<float>::pi() / 2, sinp); // use 90 degrees if out of range
    else
        angles.y() = std::asin(sinp);

    // yaw (z-axis rotation)
    double siny_cosp = 2 * (q.scalar() * q.vector().z() + q.vector().x() * q.vector().y());
    double cosy_cosp = 1 - 2 * (q.vector().y() * q.vector().y() + q.vector().z() * q.vector().z());
    angles.z() = std::atan2(siny_cosp, cosy_cosp);

    return angles;
}

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
    bool hasMesh = node.mesh.has_value();
    bool hasTexture = node.mesh.has_value() && !node.mesh->meshParts[0].textures[0].empty();
    fmt::print("{}{} {}{}\n", offset, node.name, hasMesh ? "(m)" : "", hasTexture ? "(t)" : "");

    if (!node.children.empty()) {
        for (size_t i = 0; i < node.children.size(); ++i)
            print(node.children[i], offset + " ");
    }
}

}

namespace parser
{

Magnum::Matrix4 SceneNode::computeTransformationMatrix() const {
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

Transofrmation SceneNode::computeTransformation() const
{
    float magnumScale = 1.0f / scale;
    Magnum::Vector3 magnumPosition(position.x, position.y, position.z);
    Magnum::Quaternion magnumRotation(Magnum::Vector3(rotation.x, rotation.y, rotation.z), rotation.w);
    if (!magnumRotation.isNormalized()) {
        magnumScale *= magnumRotation.dot();
        magnumRotation = magnumRotation.normalized();
    }

    Magnum::Vector3 euler = toEulerAngles(magnumRotation);

    return Transofrmation{position, Vector3{euler.x(), euler.y(), euler.z()}, magnumScale};
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
