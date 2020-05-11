#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace Utils
{

std::vector<std::string> splitString(const std::string& s, char delimiter);
std::string getFileNameWithoutExtension(const std::string& path);
std::string getFileNameWithoutExtension(const std::filesystem::path& path);

}
