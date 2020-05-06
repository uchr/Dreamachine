#pragma once

#include <filesystem>

namespace parser
{
struct SceneNode;
}

bool exportScene(const parser::SceneNode& root, const std::filesystem::path& path);
