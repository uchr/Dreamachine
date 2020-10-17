#pragma once

#include <string>
#include <vector>

namespace parser {

struct SirEntry {
    std::string filename;
    std::string sirPath;
};

struct SceneIndex {
    std::string bundleName;
    std::vector<SirEntry> sirs;
};

} // namespace parser
