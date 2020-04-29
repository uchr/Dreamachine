#include "MainWindow.h"

#include <QSurfaceFormat>
#include <QtWidgets/QApplication>

int main(int argc, char** argv) {
    Magnum::Platform::GLContext context{Magnum::NoCreate, argc, argv};
    QApplication app{argc, argv};

    QSurfaceFormat format;
    format.setVersion(4, 6);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    //w.setFormat(format);

    MainWindow mw{context};
    mw.show();

    return app.exec();
}
