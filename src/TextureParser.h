#pragma once

#include <filesystem>
#include <vector>

namespace parser
{

std::vector<std::filesystem::path> parseTextures(const std::vector<std::filesystem::path>& texturesPath, const std::filesystem::path& exportPath);

} // namespace parser
