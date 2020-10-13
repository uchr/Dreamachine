#include "MainWindow.h"
#include "View.h"

#include <parser/SceneIndex.h>
#include <parser/SceneParser.h>

#include <QMenuBar>
#include <QGridLayout>
#include <QListWidget>

#include <spdlog/spdlog.h>

MainWindow::MainWindow(Magnum::Platform::GLContext& context, const parser::SceneIndex& sceneIndex)
    : m_glView(new View(context, this, sceneIndex))
    , m_list(new QListWidget(this))
{
    m_glView->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction* open = new QAction("O&pen", fileMenu);
    fileMenu->addAction(open);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(m_list, 0, 0);
    layout->addWidget(m_glView, 0, 1);

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 4);

    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    fillList(sceneIndex);

    QWidget* window = new QWidget();
    window->setLayout(layout);
    setCentralWidget(window);

    QObject::connect(m_list, &QListWidget::itemChanged,
                     this, &MainWindow::onItemChanged);
}

void MainWindow::onItemChanged(QListWidgetItem* item)
{
    if(item->checkState() == Qt::Checked)
        m_glView->load(m_list->row(item));
    else
        m_glView->unload(m_list->row(item));

    QObject::disconnect(m_list, &QListWidget::itemChanged,
                        this, &MainWindow::onItemChanged);
    item->setBackground(QColor(item->checkState() ? "#90A4AE" : "#FFFFFF"));
    QObject::connect(m_list, &QListWidget::itemChanged,
                     this, &MainWindow::onItemChanged);
}

void MainWindow::fillList(const parser::SceneIndex& sceneIndex)
{
    QStringList strList;
    for (const auto& sir : sceneIndex.sirs)
        strList.append(QString::fromStdString(sir.filename));
    m_list->addItems(strList);

    QListWidgetItem* item = 0;
    for (int sirIndex = 0; sirIndex < m_list->count(); ++sirIndex) {
        item = m_list->item(sirIndex);
        if (canLoadItem(sceneIndex, sirIndex))
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        else
            item->setFlags(Qt::NoItemFlags);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole + 1, QVariant(sirIndex));
    }
}

bool MainWindow::canLoadItem(const parser::SceneIndex& sceneIndex, size_t sirIndex) {
    std::unique_ptr<parser::SceneParser> scene = std::make_unique<parser::SceneParser>(sceneIndex.sirs[sirIndex], sceneIndex.bundleName);
    return scene->sceneRoot.has_value();
}