#pragma once

#include "BundleHeader.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace parser {

// For parsing .bun files
class BundleParser {
public:
    BundleParser(std::filesystem::path path);
    BundleHeader parseHeader();

private:
    std::filesystem::path m_path;
};

} // namespace parser