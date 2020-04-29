#include <Corrade/Containers/Optional.h>

#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Platform/GLContext.h>
#include <Magnum/Shaders/VertexColor.h>

/* If your application is using anything from QtGui, you might get warnings
   about GLEW and errors regarding GLclampf. If that's the case, uncomment the
   following and place it as early as possible (but again *after* including
   Magnum GL headers) */
#if 0
typedef GLfloat GLclampf;
#undef __glew_h__ /* shh, Qt, shh */
#undef __GLEW_H__
#include <QtGui/qopenglfunctions.h>
#endif

#include <QtWidgets/QApplication>
#include <QtWidgets/QOpenGLWidget>

using namespace Magnum;

class DreamachineApp: public QOpenGLWidget {
    public:
        explicit DreamachineApp(Platform::GLContext& context, QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);

    private:
        void initializeGL() override;
        void paintGL() override;

        Platform::GLContext& m_context;
        std::unique_ptr<GL::Mesh> m_mesh;
        std::unique_ptr<Shaders::VertexColor2D> m_shader;
};

DreamachineApp::DreamachineApp(Platform::GLContext& context, QWidget* parent, Qt::WindowFlags f): QOpenGLWidget{parent, f}, m_context(context) {
}

void DreamachineApp::initializeGL() {
    using namespace Math::Literals;

    m_context.create();

    struct TriangleVertex {
        Vector2 position;
        Color3 color;
    };
    const TriangleVertex vertexData[]{
        {{-0.5f, -0.5f}, 0xff0000_rgbf},
        {{ 0.5f, -0.5f}, 0x00ff00_rgbf},
        {{ 0.0f,  0.5f}, 0x0000ff_rgbf}
    };

    GL::Buffer buffer;
    buffer.setData(vertexData);

    m_mesh = std::make_unique<GL::Mesh>();
    m_mesh->setCount(3).addVertexBuffer(std::move(buffer), 0,
                                       Shaders::VertexColor2D::Position{},
                                       Shaders::VertexColor2D::Color3{});

    m_shader = std::make_unique<Shaders::VertexColor2D>();

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
}

void DreamachineApp::paintGL() {
    /* Reset state to avoid Qt affecting Magnum */
    GL::Context::current().resetState(GL::Context::State::ExitExternal);

    /* Using framebuffer provided by Qt as default framebuffer */
    auto qtDefaultFramebuffer = GL::Framebuffer::wrap(defaultFramebufferObject(), {{}, {width(), height()}});

    qtDefaultFramebuffer.clear(GL::FramebufferClear::Color);

    m_mesh->draw(*m_shader);

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
}

int main(int argc, char** argv) {
    Platform::GLContext context{NoCreate, argc, argv};
    QApplication app{argc, argv};

    DreamachineApp w{context};

    w.show();

    return app.exec();
}
