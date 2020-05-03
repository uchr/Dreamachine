#include "MainWindow.h"
#include "PAKParser.h"
#include "CDRParser.h"
#include "Scene.h"
#include "MeshExporter.h"

#include <QSurfaceFormat>
#include <QtWidgets/QApplication>

#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
    const std::string bundleName = "the_gym";
    const std::filesystem::path pakPath = "package/" + bundleName +".pak";
    const std::filesystem::path sceneSDRPath = "data/generated/locations/" + bundleName + ".cdr";

    PAKParser::instance() = PAKParser(pakPath);

    CDRParser cdrParser(sceneSDRPath);
    SceneIndex sceneIndex = cdrParser.parseScene();
    //for (auto& sir : sceneIndex.sirs)
    //    std::cout << sir.filename << " : " << sir.sirPath << std::endl;

    //SirEntry selectedSir = sceneIndex.sirs[8]; //"zoe_bag"
    //Scene scene(selectedSir.sirPath, bundleName);

    for (const auto& sir : sceneIndex.sirs) {
        Scene scene(sir.sirPath, bundleName);
        std::cout << "\nExport " << sir.filename << " " << scene.meshes.size() << std::endl;
        for (size_t i = 0; i < scene.meshes.size(); ++i) {
            std::ofstream out("meshes\\" + sir.filename + "_" + std::to_string(i) + ".stl");
            std::cout << ("meshes\\" + sir.filename + "_" + std::to_string(i) + ".stl") << std::endl;
            exportSTL(scene.meshes[i], out);
        }
    }

    return 0;
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
