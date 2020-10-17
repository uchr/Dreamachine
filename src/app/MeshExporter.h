#pragma once

#include <filesystem>

namespace parser {
struct SceneNode;
}

enum class ExportMode
{
    Single,
    Multiple
};

bool exportScene(const std::vector<parser::SceneNode>& parsedSceneNodes, const std::filesystem::path& outputPath, ExportMode exportMode);
