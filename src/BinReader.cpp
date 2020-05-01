#include "BinReader.h"

#include <fstream>

BinReader::BinReader(const std::filesystem::path& path)
    : m_pos(0)
{
    std::ifstream file(path.string(), std::ios::binary);
    m_data.resize(fileSize(path));
    file.read(m_data.data(), m_data.size());
}

std::string BinReader::readStringLine() {
    std::string result;
    char c;
    while ((c = readChar()) != 0)
        result += c;
    return result;
}

int64_t BinReader::readSharkNum()
{
    int64_t num = 0;
    int n, shift = 0;
    do
    {
        n = readByte();
        num |= (int64_t)(n & 0x7f) << shift;
        shift += 7;
        if (shift >= 62)
            throw std::exception("shark numeric overflow");
    } while ((n & 0x80) != 0);
    if ((n & 0x40) != 0)
    {
        num = num - ((int64_t) 1 << shift);
    }
    return num;
}

float BinReader::readEndianFloat()
{
    unsigned char arr[4];
    for (int i = 0; i < 4; i++)
        arr[3 - i] = readByte();
    float f = *reinterpret_cast<float*>(arr); // TODO: Check
    return f;
}

char BinReader::readChar() {
    return m_data[m_pos++];
}

unsigned char BinReader::readByte() {
    return static_cast<unsigned char>(m_data[m_pos++]);
}

size_t BinReader::fileSize(const std::filesystem::path& path) const {
    std::ifstream file(path.string(), std::ios::binary);
    std::streampos begin = file.tellg();
    file.seekg (0, std::ios::end);
    std::streampos end = file.tellg();
    file.close();
    return end - begin;
}
