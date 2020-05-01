#pragma once

#include <filesystem>
#include <string>

class BinReader
{
public:
    BinReader(const std::filesystem::path& path);
    std::string readStringLine();

    int64_t readSharkNum();
    float readEndianFloat();
    char readChar();
    unsigned char readByte();

private:
    size_t fileSize(const std::filesystem::path& path) const;

    std::vector<char> m_data;
    size_t m_pos;
};
