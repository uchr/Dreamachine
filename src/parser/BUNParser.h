#pragma once

#include "BundleHeader.h"

#include <filesystem>
#include <vector>
#include <optional>

namespace parser
{

class BUNParser
{
public:
    BUNParser(std::filesystem::path path);
    BundleHeader parseHeader();

private:
    std::filesystem::path m_path;
};

}