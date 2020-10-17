#include "MainWindow.h"
#include "MeshExporter.h"

#include <parser/PAKParser.h>
#include <parser/SharkParser.h>
#include <parser/SceneParser.h>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <QSurfaceFormat>
#include <QtWidgets/QApplication>

#include <DirectXTex.h>

#include <fstream>
#include <cstdlib>

using namespace parser;

int main(int argc, char** argv) {
    const char* dreamfallTLJResPath = std::getenv("DreamfallTLJResPath");
    PAKParser::instance() = PAKParser(dreamfallTLJResPath);

    CLI::App cliapp{"Tool for extracting assets from Dreamfall: The Longest Journey"};

    bool isDebugLog = false;
    cliapp.add_flag("--debugLog", isDebugLog, "Enable debug log");
    bool isExportMode = false;
    cliapp.add_flag("--export", isExportMode, "Just export meshes without GUI");

    std::string bundleName = "japan_streets";
    cliapp.add_option("-p", bundleName, "Bundle name");

    std::string meshName = "";
    cliapp.add_option("-s", meshName, "Mesh name");

    CLI11_PARSE(cliapp, argc, argv);

    if (isDebugLog)
        spdlog::set_level(spdlog::level::debug);

    std::filesystem::path sceneSDRPath = "data/generated/locations/" + bundleName + ".cdr";
    SharkParser sceneSharkParser(sceneSDRPath);
    SceneIndex sceneIndex = sceneSharkParser.parseScene(bundleName);

    if (isExportMode) {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            spdlog::critical("DirectXTex failed initialization");
            return 1;
        }

        for (const auto& sir : sceneIndex.sirs) {
            if (sir.filename.find("anim") == 0)
                continue;

            if (!meshName.empty() && sir.filename.find(meshName) == std::string::npos)
                continue;

            SceneParser scene(sir, bundleName);

            if (scene.sceneRoot.has_value())
                spdlog::info("SIR: '{}' parsed", sir.filename);
            else 
                spdlog::warn("SIR: '{}' not parsed", sir.filename);

            if (scene.sceneRoot.has_value()) {
                std::filesystem::path path = std::filesystem::path("meshes") / bundleName;
                auto isExtracted = exportScene({*scene.sceneRoot}, path, ExportMode::Multiple);
                if (!isExtracted)
                    spdlog::error("Scene: '{}' not extracted", sir.filename);
            }
        }

        return 0;
    }
    else {
        Magnum::Platform::GLContext context{Magnum::NoCreate, argc, argv};
        QApplication app{argc, argv};

        QSurfaceFormat format;
        format.setVersion(4, 6);
        format.setProfile(QSurfaceFormat::CoreProfile);
        QSurfaceFormat::setDefaultFormat(format);

        MainWindow mw{context};
        mw.resize(1366, 768);
        mw.show();

        return app.exec();
    }
}
