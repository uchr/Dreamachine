#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace Utils
{

std::vector<std::string> splitString(const std::string& s, char delimiter);
std::string getFilenameWithoutExtension(const std::string& path);
std::string getFilenameWithoutExtension(const std::filesystem::path& path);

}
