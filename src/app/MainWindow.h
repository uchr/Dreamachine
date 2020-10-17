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
class BundleListWindow;

QT_FORWARD_DECLARE_CLASS(QListWidget)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Magnum::Platform::GLContext& context);
    ~MainWindow();

    void onItemChanged(QListWidgetItem* item);

    void doExportAsSingleMesh();
    void doExportAsMultipleMeshes();

    void showSelectBundleWindow();

    void loadBundle(const std::string& bundleName);

private:
    void fillList();
    bool canLoadItem(size_t sirIndex);
    std::vector<parser::SceneNode> loadedMeshes();

    QListWidget* m_list;
    View* m_glView;

    BundleListWindow* m_bundleListWindow;

    std::unique_ptr<parser::SceneIndex> m_sceneIndex;
};