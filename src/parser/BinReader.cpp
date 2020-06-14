#include "BinReader.h"
#include "PAKParser.h"

#include <cpp-mmf/memory_mapped_file.hpp>

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
    m_mmf = std::make_unique<memory_mapped_file::read_only_mmf>(path.string().c_str(), true);
    if (!m_mmf->is_open())
        spdlog::error("Can't open {}", path.string());
}

BinReaderMmap::BinReaderMmap(const std::filesystem::path& path, size_t offset, size_t size) {
    m_mmf = std::make_unique<memory_mapped_file::read_only_mmf>(path.string().c_str(), false);
    m_mmf->map(offset, size);
    if (!m_mmf->is_open())
        spdlog::error("Can't open {}", path.string());
}

BinReaderMmap::~BinReaderMmap() = default;

const char* BinReaderMmap::data() const {
    return m_mmf->data();
}

char* BinReaderMmap::data() {
    return const_cast<char*>(m_mmf->data());
}

size_t BinReaderMmap::size() const {
    return m_mmf->mapped_size();
}

bool BinReaderMmap::isOpen() const {
    return m_mmf->is_open();
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