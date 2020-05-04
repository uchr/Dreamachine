#pragma once

#include <vector>
#include <string>

namespace Utils
{

std::vector<std::string> splitString(const std::string& s, char delimiter);
std::string getFileNameWithoutExtension(const std::string& path);

}
