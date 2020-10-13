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

private:
    void fillList(const parser::SceneIndex& sceneIndex);
    bool canLoadItem(const parser::SceneIndex& sceneIndex, size_t sirIndex);

    QListWidget* m_list;
    View* m_glView;
};