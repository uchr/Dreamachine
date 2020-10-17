#pragma once

#include "BundleHeader.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace parser {

class BUNParser {
public:
    BUNParser(std::filesystem::path path);
    BundleHeader parseHeader();

private:
    std::filesystem::path m_path;
};

} // namespace parser