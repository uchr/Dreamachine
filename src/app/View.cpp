#include "View.h"
#include "ViewScene.h"

#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Framebuffer.h>

#include <spdlog/spdlog.h>

using namespace Magnum;
using namespace Math::Literals;

View::View(Platform::GLContext& context, QWidget* parent)
    : QOpenGLWidget(parent)
    , m_context(context)
{
}

View::~View() = default;

void View::load(size_t sirIndex) {
    m_meshToLoading.push_back(sirIndex);
}

void View::unload(size_t sirIndex) {
    m_meshToUnloading.push_back(sirIndex);
}

void View::setSceneIndex(parser::SceneIndex* sceneIndex) {
    m_sceneIndex = sceneIndex;
    m_viewScene->setSceneIndex(sceneIndex);
}

void View::initializeGL() {
    m_context.create();

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    m_viewScene = std::make_unique<ViewScene>(m_inputManager, m_timeManager);
    m_timeManager.init();

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);
}

void View::paintGL() {
    /* Reset state to avoid Qt affecting Magnum */
    GL::Context::current().resetState(GL::Context::State::ExitExternal);
    auto qtDefaultFramebuffer = GL::Framebuffer::wrap(defaultFramebufferObject(), {{}, {width(), height()}});
    qtDefaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    while(!m_meshToLoading.empty()) {
        size_t sirIndex = m_meshToLoading.back();
        m_meshToLoading.pop_back();

        m_viewScene->load(sirIndex);
    }

    while(!m_meshToUnloading.empty()) {
        size_t sirIndex = m_meshToUnloading.back();
        m_meshToUnloading.pop_back();

        m_viewScene->unload(sirIndex);
    }

    m_viewScene->draw();
    m_timeManager.update();

    /* Clean up Magnum state when giving control back to Qt */
    GL::Context::current().resetState(GL::Context::State::EnterExternal);

    update();
}

void View::resizeGL(int width, int height) {
    m_viewScene->setViewport(width, height);
}

void View::mousePressEvent(QMouseEvent* event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;

    m_inputManager.mouseMoveStart(event->localPos().x(), event->localPos().y());
}

void View::mouseReleaseEvent(QMouseEvent* event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;
}

void View::mouseMoveEvent(QMouseEvent* event) {
    if (!event->buttons().testFlag(Qt::LeftButton))
        return;

    m_inputManager.mouseMove(event->localPos().x(), event->localPos().y());
    m_viewScene->rotateCamera(m_inputManager.mouseDelta(width(), height()));
}

void View::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat())
        return;

    m_inputManager.keyPress(event->key());
}

void View::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat())
        return;

    m_inputManager.keyRelease(event->key());
}
