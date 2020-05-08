#include "MainWindow.h"
#include "GLView.h"
#include "SceneIndex.h"

#include <QMenuBar>
#include <QGridLayout>

MainWindow::MainWindow(Magnum::Platform::GLContext& context, const parser::SceneIndex& sceneIndex)
    : m_glView(new GLView(context, this, sceneIndex))
{
    m_glView->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *open = new QAction("O&pen", fileMenu);
    fileMenu->addAction(open);

    QGridLayout* layout = new QGridLayout(this);
    m_listView = new QListView(this);
    layout->addWidget(m_listView, 0, 0);
    layout->addWidget(m_glView, 0, 1);

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 4);

    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    QWidget *window = new QWidget();
    window->setLayout(layout);
    setCentralWidget(window);
}