#include "BinReader.h"
#include "PAKParser.h"

#include <spdlog/spdlog.h>

#include <fstream>

namespace parser
{

std::string BinReader::readStringLine() {
    std::string result;
    char c;
    while ((c = readChar()) != 0)
        result += c;
    return result;
}

std::string BinReader::readString(size_t length) {
    std::vector chars = readChars(length); // TODO: Rewrite
    std::string result(chars.begin(), chars.end());
    return result;
}

std::vector<char> BinReader::readChars(size_t length) {
    std::vector<char> result(data() + m_pos, data() + m_pos + length);
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
    float value = *reinterpret_cast<float*>(data() + m_pos);
    m_pos += sizeof(float);
    return value;
}

uint32_t BinReader::readUint32() {
    uint32_t value = *reinterpret_cast<uint32_t*>(data() + m_pos);
    m_pos += sizeof(uint32_t);
    return value;
}

int32_t BinReader::readInt32() {
    int32_t value = *reinterpret_cast<int32_t*>(data() + m_pos);
    m_pos += sizeof(int32_t);
    return value;
}

uint16_t BinReader::readUint16() {
    uint16_t value = *reinterpret_cast<uint16_t*>(data() + m_pos);
    m_pos += sizeof(uint16_t);
    return value;
}

char BinReader::readChar() {
    return data()[m_pos++];
}

byte BinReader::readByte() {
    return static_cast<byte>(data()[m_pos++]);
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
}

bool BinReader::isEnd() const {
    return m_pos == size();
}

BinReaderMmap::BinReaderMmap(const std::filesystem::path& path) {
    mmf.open(path.string().c_str(), true);
    if (!mmf.is_open())
        spdlog::error("Can't open {}", path.string());
}

BinReaderMmap::BinReaderMmap(const std::filesystem::path& path, size_t offset, size_t size) {
    mmf.open(path.string().c_str(), false);
    mmf.map(offset, size);
    if (!mmf.is_open())
        spdlog::error("Can't open {}", path.string());
}

const char* BinReaderMmap::data() const {
    return mmf.data();
}

char* BinReaderMmap::data() {
    return const_cast<char*>(mmf.data());
}

size_t BinReaderMmap::size() const {
    return mmf.mapped_size();
}

bool BinReaderMmap::isOpen() const {
    return mmf.is_open();
}

BinReaderMemory::BinReaderMemory(const char* data, size_t size)
    : m_data(data)
    , m_size(size)
{
    assert(data != nullptr);
    assert(size > 0);
}

const char* BinReaderMemory::data() const {
    return m_data;
}

char* BinReaderMemory::data() {
    return const_cast<char*>(m_data);
}

size_t BinReaderMemory::size() const {
    return m_size;
}

}