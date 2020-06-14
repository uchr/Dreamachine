#pragma once

#include "InputManager.h"

#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Platform/GLContext.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/Phong.h>

#pragma warning(push)
#pragma warning(disable : 5054)
#include <QListView>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QElapsedTimer>
#pragma warning(pop)

namespace parser
{
struct SceneIndex;
struct SceneNode;
}

typedef Magnum::SceneGraph::Object<Magnum::SceneGraph::MatrixTransformation3D> Object3D;
typedef Magnum::SceneGraph::Scene<Magnum::SceneGraph::MatrixTransformation3D> Scene3D;

class GLView: public QOpenGLWidget {
    public:
        explicit GLView(Magnum::Platform::GLContext& context, QWidget* parent, const parser::SceneIndex& sceneIndex);

    private:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int width, int height) override;

        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;

        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;

        void updateCameraObject();
        void setupScene(const parser::SceneNode& node);
        void setupScene(const parser::SceneNode& node, Object3D& parent, size_t& meshIndex);

        InputManager m_inputManager;

        Magnum::Platform::GLContext& m_context;
        Magnum::Vector2 m_previousCursorPosition;

        std::unique_ptr<Magnum::Shaders::Phong> m_texturedShader;

        Magnum::Containers::Array<Magnum::Containers::Optional<Magnum::GL::Mesh>> m_meshes;
        Magnum::Containers::Array<Magnum::Containers::Optional<Magnum::GL::Texture2D>> m_textures;

        Scene3D m_scene;
        Object3D m_manipulator, m_cameraObject;
        Magnum::SceneGraph::Camera3D* m_camera;
        Magnum::SceneGraph::DrawableGroup3D m_drawables;

        float m_phi;
        float m_theta;
        Magnum::Vector3 m_cameraPosition;

        QElapsedTimer m_time;
        float m_deltaTime = 0.0f;
        int m_currentTime = 0;
        int m_oldTime = 0;

        const parser::SceneIndex& m_sceneIndex;

        const float CameraSpeed = 20.0f;
};
