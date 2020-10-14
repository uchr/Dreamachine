#pragma once

#include <Magnum/Platform/GLContext.h>

#pragma warning(push)
#pragma warning(disable : 5054)
#include <QObject>
#include <QMainWindow>
#pragma warning(pop)

namespace parser
{
struct SceneIndex;
struct SceneNode;
}
class View;

QT_FORWARD_DECLARE_CLASS(QListWidget)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Magnum::Platform::GLContext& context, const parser::SceneIndex& sceneIndex);

    void onItemChanged(QListWidgetItem* item);
    void doExportAsSingleMesh();
    void doExportAsMultipleMeshes();

private:
    void fillList();
    bool canLoadItem(size_t sirIndex);
    std::vector<parser::SceneNode> loadedMeshes();

    QListWidget* m_list;
    View* m_glView;

    const parser::SceneIndex& m_sceneIndex;
};