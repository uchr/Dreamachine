#pragma once

#include <cpp-mmf/memory_mapped_file.hpp>

#include <filesystem>
#include <string>

namespace parser
{

using byte = unsigned char;

class BinReader
{
public:
    template <typename T>
    T read()
    {
        T value = *reinterpret_cast<T*>(data() + m_pos);
        m_pos += sizeof(T);
        return value;
    }

    std::string readStringLine();
    std::string readString(size_t length);
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

    virtual const char* data() const = 0;
    virtual char* data() = 0;

    virtual size_t size() const = 0;

private:
    size_t m_pos = 0;
    size_t m_posZero = 0;
};

class BinReaderMmap : public BinReader {
public:
    BinReaderMmap(const std::filesystem::path& path);
    BinReaderMmap(const std::filesystem::path& path, size_t offset, size_t size);

    const char* data() const override;
    char* data() override;

    size_t size() const override;

    bool isOpen() const;

private:
    memory_mapped_file::read_only_mmf mmf;
};

class BinReaderMemory : public BinReader {
public:
    BinReaderMemory(const char* data, size_t size);

    const char* data() const override;
    char* data() override;

    size_t size() const override;

private:
    const char* m_data;
    size_t m_size;
};

}