#pragma once

#include <QObject>
#include <QWidget>

class MainWindow;

QT_FORWARD_DECLARE_CLASS(QListWidget)

class BundleListWindow : public QWidget {
   Q_OBJECT

public:
    BundleListWindow(MainWindow* mainWindow);

    void doLoadBundle();

private:
    void fillBundleList();

    QListWidget* m_list;
    MainWindow* m_mainWindow;
};
