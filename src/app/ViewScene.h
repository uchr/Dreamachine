#pragma once

#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/Phong.h>

namespace parser
{
struct SceneIndex;
struct SceneNode;
}

class TimeManager;
class InputManager;

using Object3D = Magnum::SceneGraph::Object<Magnum::SceneGraph::MatrixTransformation3D>;
using Scene3D = Magnum::SceneGraph::Scene<Magnum::SceneGraph::MatrixTransformation3D>;

class ViewScene {
public:
    ViewScene(const parser::SceneIndex& sceneIndex, const InputManager& inputManager, const TimeManager& timeManager);

    void load(size_t meshIndex);
    void draw();
    void setViewport(int width, int height);
    void rotateCamera(const Magnum::Vector2& mouseDelta);

private:
    struct CameraTransform {
        float phi = 3.14f;
        float theta = 1.3f;
        Magnum::Vector3 position = Magnum::Vector3(0.5f, 32.0f, 8.0f);
    };

    void updateCameraTransform();

    void setupScene(const parser::SceneNode& node);
    void setupScene(const parser::SceneNode& node, Object3D& parent, size_t& meshIndex);

    Magnum::Shaders::Phong m_texturedShader;

    Magnum::Containers::Array<Magnum::Containers::Optional<Magnum::GL::Mesh>> m_meshes;
    Magnum::Containers::Array<Magnum::Containers::Optional<Magnum::GL::Texture2D>> m_textures;

    Scene3D m_scene;
    Object3D m_manipulator, m_cameraObject;
    Magnum::SceneGraph::Camera3D* m_camera;
    Magnum::SceneGraph::DrawableGroup3D m_drawables;

    CameraTransform m_cameraTransform;

    std::unordered_map<int, bool> m_loadedMeshes;

    const TimeManager& m_timeManager;
    const InputManager& m_inputManager;
    const parser::SceneIndex& m_sceneIndex;

    const float CameraMovementSpeed = 20.0f;
    const float CameraRotationSpeed = 3.0f;
};