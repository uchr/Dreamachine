#include "BinReader.h"
#include "PAKParser.h"

#include <fstream>

namespace parser
{

BinReader::BinReader(const std::filesystem::path& path)
    : m_pos(0)
{
    std::ifstream file(path.string(), std::ios::binary);
    if (!file.is_open())
        PAKParser::instance().tryExtract(path);
    file = std::ifstream(path.string(), std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("File '" + path.string() + "' not found");
    m_data.resize(fileSize(path));
    file.read(m_data.data(), m_data.size());
}

BinReader::BinReader(std::vector<char> data)
    : m_data(std::move(data))
    , m_pos(0)
{
}

std::string BinReader::readStringLine() {
    std::string result;
    char c;
    while ((c = readChar()) != 0)
        result += c;
    return result;
}

std::string BinReader::readString(size_t length) {
    std::string result(m_data.begin() + m_pos, m_data.begin() + m_pos + length);
    m_pos += length;
    return result;
}

std::vector<char> BinReader::readChars(size_t length) {
    std::vector<char> result(m_data.begin() + m_pos, m_data.begin() + m_pos + length);
    m_pos += length;
    return result;
}

std::vector<uint32_t> BinReader::readUint32Table(int length, uint32_t pos) {
    Assert(pos);
    std::vector<uint32_t> table(length);
    for (int i = 0; i < length; i++)
        table[i] = readUint32();
    return table;
}

std::vector<int32_t> BinReader::readInt32Table(int length, uint32_t pos) {
    Assert(pos);
    std::vector<int32_t> table(length);
    for (int i = 0; i < length; i++)
        table[i] = readInt32();
    return table;
}

std::vector<uint16_t> BinReader::readUint16Table(int length, uint32_t pos) {
    Assert(pos);
    std::vector<uint16_t> table(length);
    for (int i = 0; i < length; i++)
        table[i] = readUint16();
    return table;
}

std::vector<float> BinReader::readFloatTable(int length, uint32_t pos)
{
    Assert(pos);
    std::vector<float> table(length);
    for (int i = 0; i < length; i++)
        table[i] = readFloat();
    return table;
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
    byte arr[4];
    for (int i = 0; i < 4; i++)
        arr[3 - i] = readByte();
    float f = *reinterpret_cast<float*>(arr); // TODO: Check
    return f;
}

float BinReader::readFloat()
{
    float value = *reinterpret_cast<float*>(m_data.data() + m_pos);
    m_pos += sizeof(float);
    return value;
}

uint32_t BinReader::readUint32() {
    uint32_t value = *reinterpret_cast<uint32_t*>(m_data.data() + m_pos);
    m_pos += sizeof(uint32_t);
    return value;
}

int32_t BinReader::readInt32() {
    int32_t value = *reinterpret_cast<int32_t*>(m_data.data() + m_pos);
    m_pos += sizeof(int32_t);
    return value;
}

uint16_t BinReader::readUint16() {
    uint16_t value = *reinterpret_cast<uint16_t*>(m_data.data() + m_pos);
    m_pos += sizeof(uint16_t);
    return value;
}

char BinReader::readChar() {
    return m_data[m_pos++];
}

byte BinReader::readByte() {
    return static_cast<byte>(m_data[m_pos++]);
}

size_t BinReader::getPosition() const {
    return m_pos;
}

void BinReader::setPosition(size_t newPosition) {
    m_pos = newPosition;
}

void BinReader::shiftPosition(int64_t offset) {
    m_pos = static_cast<int64_t>(m_pos) + offset;
}

void BinReader::checkPosition(size_t expectPosition) {
    if (expectPosition > 0 && expectPosition != m_pos)
        throw std::exception("Not expected pos");
}

void BinReader::setZeroPos(size_t pos)
{
    m_posZero = pos;
}

void BinReader::resetZeroPos()
{
    m_posZero = m_pos;
}

bool BinReader::IsPos(size_t npos)
{
    size_t pos = npos == 0 ? 0 : npos + m_posZero;
    return pos == m_pos;
}

void BinReader::Assert(size_t npos)
{
    Assert0 (npos == 0 ? 0 : npos + m_posZero);
}

void BinReader::Assert0(size_t pos)
{
    if (pos > 0 && m_pos != pos)
        throw new std::runtime_error(std::string("m_posZero: ") + std::to_string(m_posZero) + " pos: " + std::to_string(pos));
        //std::cerr << std::to_string(m_pos) + " != " + std::to_string(pos) << std::endl;
}

bool BinReader::isEnd() const {
    return m_pos == m_data.size();
}

const char* BinReader::data() const {
    return m_data.data();
}

size_t BinReader::fileSize(const std::filesystem::path& path) const {
    std::ifstream file(path.string(), std::ios::binary);
    std::streampos begin = file.tellg();
    file.seekg (0, std::ios::end);
    std::streampos end = file.tellg();
    file.close();
    return end - begin;
}

}