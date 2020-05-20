#include "MainWindow.h"
#include "PAKParser.h"
#include "SharkParser.h"
#include "Scene.h"
#include "MeshExporter.h"

#include <spdlog/spdlog.h>

#include <QSurfaceFormat>
#include <QtWidgets/QApplication>

#include <fstream>
#include <cstdlib>

using namespace parser;

int main(int argc, char** argv) {
    //spdlog::set_level(spdlog::level::debug);

    const char* dreamfallTLJResPath = std::getenv("DreamfallTLJResPath");

    const std::string bundleName = "japan_streets";
    const std::filesystem::path sceneSDRPath = "data/generated/locations/" + bundleName + ".cdr";

    PAKParser::instance() = PAKParser(dreamfallTLJResPath);

    SharkParser sceneSDRParser(sceneSDRPath);
    SceneIndex sceneIndex = sceneSDRParser.parseScene(bundleName);

    Magnum::Platform::GLContext context{Magnum::NoCreate, argc, argv};
    QApplication app{argc, argv};

    QSurfaceFormat format;
    format.setVersion(4, 6);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    MainWindow mw{context, sceneIndex};
    mw.resize(1366, 768);
    mw.show();

    for (const auto& sir : sceneIndex.sirs) { // the_gym: 4, 8
        Scene scene(sir, bundleName);

        if (scene.sceneRoot.has_value())
            spdlog::info("SIR: '{}' parsed", sir.filename);
        else 
            spdlog::warn("SIR: '{}' not parsed", sir.filename);

        if (scene.sceneRoot.has_value()) {
            auto isExtracted = exportScene(*scene.sceneRoot, bundleName, sir.filename);
            if (!isExtracted)
                spdlog::error("Scene: '{}' not extracted", sir.filename);
        }
    }

    return app.exec();
}
