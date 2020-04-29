#pragma once

#include <Magnum/Platform/GLContext.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

#include <QListView>
#include <QMouseEvent>
#include <QOpenGLWidget>

class GLView: public QOpenGLWidget {
    public:
        explicit GLView(Magnum::Platform::GLContext& context, QWidget* parent);

    private:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int w, int h) override;

        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;

        Magnum::Platform::GLContext& m_context;
        std::unique_ptr<Magnum::GL::Mesh> m_mesh;
        std::unique_ptr<Magnum::Shaders::Phong> m_shader;

        Magnum::Matrix4 m_transformation, m_projection;
        Magnum::Color3 m_color;
        Magnum::Vector2 m_prevPos;
};
