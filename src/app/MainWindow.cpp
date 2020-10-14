#include "MainWindow.h"
#include "MeshExporter.h"
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
    , m_sceneIndex(sceneIndex)
{
    m_glView->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction* open = new QAction("Open", fileMenu);
    fileMenu->addAction(open);
    
    QAction* exportAsSingleMeshAction = new QAction("Export as single mesh", fileMenu);
    fileMenu->addAction(exportAsSingleMeshAction);
    connect(exportAsSingleMeshAction, &QAction::triggered, this, &MainWindow::doExportAsSingleMesh);

    QAction* exportAsMultipleAction = new QAction("Export as multiple meshes", fileMenu);
    fileMenu->addAction(exportAsMultipleAction);
    connect(exportAsMultipleAction, &QAction::triggered, this, &MainWindow::doExportAsMultipleMeshes);

    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(m_list, 0, 0);
    layout->addWidget(m_glView, 0, 1);

    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 4);

    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    fillList();

    QWidget* window = new QWidget();
    window->setLayout(layout);
    setCentralWidget(window);

    QObject::connect(m_list, &QListWidget::itemChanged, this, &MainWindow::onItemChanged);
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

void MainWindow::doExportAsSingleMesh() {
    std::vector<parser::SceneNode> parsedSceneNodes = loadedMeshes();
    if (parsedSceneNodes.empty())
        return;

    std::filesystem::path path = std::filesystem::path("meshes") / (m_sceneIndex.bundleName + ".fbx");
    auto isExtracted = exportScene(parsedSceneNodes, path, ExportMode::Single);
    if (!isExtracted)
        spdlog::error("Export single mesh failed");
}

void MainWindow::doExportAsMultipleMeshes() {
    std::vector<parser::SceneNode> parsedSceneNodes = loadedMeshes();
    if (parsedSceneNodes.empty())
        return;

    std::filesystem::path path = std::filesystem::path("meshes") / m_sceneIndex.bundleName;
    auto isExtracted = exportScene(parsedSceneNodes, path, ExportMode::Multiple);
    if (!isExtracted)
        spdlog::error("Export multiple meshes failed");
}

void MainWindow::fillList()
{
    QStringList strList;
    for (const auto& sir : m_sceneIndex.sirs)
        strList.append(QString::fromStdString(sir.filename));
    m_list->addItems(strList);

    for (int sirIndex = 0; sirIndex < m_list->count(); ++sirIndex) {
        QListWidgetItem* item = m_list->item(sirIndex);
        if (canLoadItem(sirIndex))
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        else
            item->setFlags(Qt::NoItemFlags);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole + 1, QVariant(sirIndex));
    }
}

bool MainWindow::canLoadItem(size_t sirIndex) {
    std::unique_ptr<parser::SceneParser> scene = std::make_unique<parser::SceneParser>(m_sceneIndex.sirs[sirIndex], m_sceneIndex.bundleName);
    return scene->sceneRoot.has_value();
}

std::vector<parser::SceneNode> MainWindow::loadedMeshes() {
    std::vector<parser::SceneNode> parsedSceneNodes;
    for (int sirIndex = 0; sirIndex < m_list->count(); ++sirIndex) {
        QListWidgetItem* item = m_list->item(sirIndex);
        if (!item->checkState())
            continue;

        auto sir = m_sceneIndex.sirs[sirIndex];
        spdlog::info(item->text().toStdString());

        parser::SceneParser scene(sir, m_sceneIndex.bundleName);

        if (scene.sceneRoot.has_value())
            spdlog::info("SIR: '{}' parsed", sir.filename);
        else 
            spdlog::warn("SIR: '{}' not parsed", sir.filename);

        if (scene.sceneRoot.has_value())
            parsedSceneNodes.push_back(*scene.sceneRoot);
    }
    return parsedSceneNodes;
}