#include "MainWindow.h"
#include "PAKParser.h"

#include <QSurfaceFormat>
#include <QtWidgets/QApplication>

int main(int argc, char** argv) {
    PAKParser pakParser("package/the_gym.pak");

    pakParser.extractCDR();

    return 0;

    Magnum::Platform::GLContext context{Magnum::NoCreate, argc, argv};
    QApplication app{argc, argv};

    QSurfaceFormat format;
    format.setVersion(4, 6);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow mw{context};
    mw.show();

    return app.exec();
}
