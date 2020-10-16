#include "BundleListWindow.h"

#include "MainWindow.h"

#include "parser/CommonPath.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>

BundleListWindow::BundleListWindow(MainWindow* mainWindow)
    : m_mainWindow(mainWindow)
{
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_list = new QListWidget(this);
    QObject::connect(m_list, &QListWidget::itemDoubleClicked, this, &BundleListWindow::doLoadBundle);
    layout->addWidget(m_list);

    auto* loadButton = new QPushButton("Load");
    loadButton->setFixedWidth(100);
    QObject::connect(loadButton, &QPushButton::clicked, this, &BundleListWindow::doLoadBundle);
    layout->addWidget(loadButton);
    layout->setAlignment(loadButton, Qt::AlignCenter);

    setLayout(layout);

    fillBundleList();
}

void BundleListWindow::doLoadBundle() {
    auto selectedItems = m_list->selectedItems();
    if (selectedItems.empty())
        return;
    std::filesystem::path bundlePath(selectedItems.front()->text().toStdString());
    m_mainWindow->loadBundle(bundlePath.filename().replace_extension("").string());
    this->hide();
}

void BundleListWindow::fillBundleList() {
    QStringList strList;
    for(auto& bundlePath : std::filesystem::directory_iterator(bundlesFolderPath))
        strList.append(QString::fromStdString(bundlePath.path().string()));
    m_list->addItems(strList);
}
