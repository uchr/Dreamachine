#pragma once

#include "InputManager.h"
#include "TimeManager.h"

#include <Magnum/Platform/GLContext.h>

#pragma warning(push)
#pragma warning(disable : 5054)
#include <QListView>
#include <QMouseEvent>
#include <QOpenGLWidget>
#pragma warning(pop)

namespace parser
{
struct SceneIndex;
}
class ViewScene;

class View: public QOpenGLWidget {
public:
    explicit View(Magnum::Platform::GLContext& context, QWidget* parent, const parser::SceneIndex& sceneIndex);
    ~View();

private:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    std::unique_ptr<ViewScene> m_viewScene;
    InputManager m_inputManager;
    TimeManager m_timeManager;

    Magnum::Platform::GLContext& m_context;

    const parser::SceneIndex& m_sceneIndex;
};
