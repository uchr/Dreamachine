#pragma once

#include <string>
#include <vector>

struct SirEntry
{
    std::string filename;
    std::string sirPath;
};

struct SceneIndex
{
    std::string bundle;
    std::vector<SirEntry> sirs;
};
