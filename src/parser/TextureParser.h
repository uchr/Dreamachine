#pragma once

#include <filesystem>
#include <vector>

namespace parser {

struct MeshPart;

void parseTextures(MeshPart& meshPart, const std::vector<std::filesystem::path>& texturesPath, const std::filesystem::path& exportFolder);

} // namespace parser
