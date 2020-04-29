#pragma once
#include <Magnum/Platform/GLContext.h>

#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QOpenGLWidget)
QT_FORWARD_DECLARE_CLASS(QListView)

class MainWindow : public QMainWindow
{
public:
    MainWindow(Magnum::Platform::GLContext& context);

    QListView* m_listView;
    QOpenGLWidget* m_glView;
};