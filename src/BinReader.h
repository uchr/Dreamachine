#pragma once

#include <filesystem>
#include <string>

using byte = unsigned char;

class BinReader
{
public:
    BinReader(const std::filesystem::path& path);
    BinReader(std::vector<char> data);

    template <typename T>
    T read()
    {
        T value = *reinterpret_cast<T*>(m_data.data() + m_pos);
        m_pos += sizeof(T);
        return value;
    }

    std::string readStringLine();
    std::vector<char> readChars(size_t length);

    std::vector<uint32_t> readUint32Table(int length, uint32_t pos);
    std::vector<int32_t> readInt32Table(int length, uint32_t pos);

    std::vector<uint16_t> readUint16Table(int length, uint32_t pos);

    std::vector<float> readFloatTable(int length, uint32_t pos);

    int64_t readSharkNum();

    float readEndianFloat();
    float readFloat();

    uint32_t readUint32();
    int32_t readInt32();

    uint16_t readUint16();

    char readChar();
    byte readByte();

    size_t getPosition() const;
    void setPosition(size_t newPosition);
    void shiftPosition(int64_t offset);
    void checkPosition(size_t expectPosition);

    void setZeroPos(size_t pos);
    void resetZeroPos();
    bool IsPos(size_t npos);
    void Assert(size_t npos);
    void Assert0(size_t pos);

    bool isEnd() const;

private:
    size_t fileSize(const std::filesystem::path& path) const;

    std::vector<char> m_data;
    size_t m_pos;

    size_t m_posZero;
};
