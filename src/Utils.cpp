#include "Utils.h"

#include <sstream>
#include <filesystem>

namespace Utils
{

std::vector<std::string> splitString(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter))
    {
       tokens.push_back(token);
    }
    return tokens;
}

std::string getFileNameWithoutExtension(const std::string& path) {
    std::filesystem::path temp(path);
    return temp.filename().replace_extension("").string();
}

}