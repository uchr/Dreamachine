#include "Utils.h"

#include <filesystem>
#include <sstream>

namespace Utils {

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string getFilenameWithoutExtension(const std::string& path) {
    std::filesystem::path temp(path);
    return temp.filename().replace_extension().string();
}

std::string getFilenameWithoutExtension(const std::filesystem::path& path) {
    return path.filename().replace_extension().string();
}

} // namespace Utils