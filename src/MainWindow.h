#pragma once

#include <Magnum/Platform/GLContext.h>

#pragma warning(push)
#pragma warning(disable : 5054)
#include <QMainWindow>
#pragma warning(pop)

namespace parser
{
struct SceneIndex;
}

QT_FORWARD_DECLARE_CLASS(QOpenGLWidget)
QT_FORWARD_DECLARE_CLASS(QListView)

class MainWindow : public QMainWindow
{
public:
    MainWindow(Magnum::Platform::GLContext& context, const parser::SceneIndex& sceneIndex);

    QListView* m_listView;
    QOpenGLWidget* m_glView;
};