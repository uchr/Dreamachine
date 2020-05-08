#include "MainWindow.h"
#include "PAKParser.h"
#include "SharkParser.h"
#include "Scene.h"
#include "MeshExporter.h"

#include <QSurfaceFormat>
#include <QtWidgets/QApplication>

#include <iostream>
#include <fstream>

using namespace parser;

int main(int argc, char** argv) {
    const std::string bundleName = "the_gym";
    const std::filesystem::path pakPath = "package/" + bundleName +".pak";
    const std::filesystem::path sceneSDRPath = "data/generated/locations/" + bundleName + ".cdr";

    PAKParser::instance() = PAKParser(pakPath);

    SharkParser sceneSDRParser(sceneSDRPath);
    SceneIndex sceneIndex = sceneSDRParser.parseScene(bundleName);

    /*
    for (const auto& sir : sceneIndex.sirs) {
        Scene scene(sir.sirPath, bundleName);

        std::cout << "\n" << sir.filename << ":" << std::endl;
        if (scene.sceneRoot.has_value())
            std::cout << "Parsed successfully" << std::endl;
        else 
            std::cout << "Parsed unsuccessfully" << std::endl;

        if (scene.sceneRoot.has_value()) {
            std::filesystem::path meshPath = "meshes\\" + sir.filename + ".fbx";
            auto isExtracted = exportScene(*scene.sceneRoot, meshPath);
            if (isExtracted)
                std::cout << "Extracted successfully" << std::endl;
            else 
                std::cout << "Extracted unsuccessfully" << std::endl;
        }
    }
    */

    Magnum::Platform::GLContext context{Magnum::NoCreate, argc, argv};
    QApplication app{argc, argv};

    QSurfaceFormat format;
    format.setVersion(4, 6);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow mw{context, sceneIndex};
    mw.resize(1366, 768);
    mw.show();

    return app.exec();
}
