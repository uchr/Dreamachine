#pragma once

#include <filesystem>

struct SceneNode;

bool exportScene(const SceneNode& root, const std::filesystem::path& path);
