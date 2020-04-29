#include <Corrade/Containers/Optional.h>

#include <Magnum/GL/Framebuffer.h>
#include <Magnum/Platform/GLContext.h>

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

        Platform::GLContext& _context;
};

DreamachineApp::DreamachineApp(Platform::GLContext& context, QWidget* parent, Qt::WindowFlags f): QOpenGLWidget{parent, f}, _context(context) {
    /* TODO: Add your context format setup code here */
}

void DreamachineApp::initializeGL() {
    _context.create();

    /* TODO: Add your initialization code here */

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
}

void DreamachineApp::paintGL() {
    /* Reset state to avoid Qt affecting Magnum */
    GL::Context::current().resetState(GL::Context::State::ExitExternal);

    /* Using framebuffer provided by Qt as default framebuffer */
    auto qtDefaultFramebuffer = GL::Framebuffer::wrap(defaultFramebufferObject(), {{}, {width(), height()}});

    qtDefaultFramebuffer.clear(GL::FramebufferClear::Color);

    /* TODO: Add your drawing code here */

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
