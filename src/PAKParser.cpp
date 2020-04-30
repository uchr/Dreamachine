#include "PAKParser.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

const std::vector<char> charTable{'\0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
                                  'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
                                  'x', 'y', 'z', '\\', '?', '?', '-', '_', '\'', '.', '0', '1',
                                  '2', '3', '4', '5', '6', '7', '8', '9'};

uint32_t readUint32(char* data, size_t& pos)
{
    uint32_t value = *reinterpret_cast<uint32_t*>(data + pos);
    pos += sizeof(uint32_t);
    return value;
}

int32_t readInt32(char* data, size_t& pos)
{
    int32_t value = *reinterpret_cast<int32_t*>(data + pos);
    pos += sizeof(int32_t);
    return value;
}

char hex2char(int a) {
    if (a > charTable.size())
        return '?';
    return charTable[a];
}

int char2hex(char c)
{
    auto it = std::find(charTable.begin(), charTable.end(), c);
    if (it == charTable.end())
        return -1;
    return static_cast<int>(std::distance(charTable.begin(), it));
}

} // anonymouse namespace

struct FileEntry
{
    uint32_t offset, oldOffset;
    int32_t size;
    int32_t hOffset, hLen, hRef;
    std::string partialName = "";

    FileEntry(char* data, size_t& pos);
    void fillIn(const std::vector<char>& nameBlock);
    bool isRealFile() const;
};

FileEntry::FileEntry(char* data, size_t& pos) {
    offset = readUint32(data, pos);
    size = readInt32(data, pos);
    hOffset = readInt32(data, pos);
    hLen = readInt32(data, pos);
    hRef = readInt32(data, pos);
    if (isRealFile())
        --hLen;
}

void FileEntry::fillIn(const std::vector<char>& nameBlock)
{
    for (int i = hRef; i < nameBlock.size(); i++)
    {
        if (nameBlock[i] == 0)
            break;
        partialName += nameBlock[i];
    }
}

bool FileEntry::isRealFile() const
{
    return size > 0;
}

PAKParser::PAKParser(std::filesystem::path path) 
    : m_path(std::move(path))
{
    parse();
}

PAKParser::~PAKParser() = default;

void PAKParser::extractCDR() const {
    std::string filename = m_path.filename().string();
    filename = std::string(filename.begin(), filename.begin() + (filename.size() - 4));
    const std::string innerPath = "data\\generated\\locations\\" + filename + ".cdr";

    std::cout << "Try to extract '" << filename << "' by '" << innerPath << "'" << std::endl;

    const FileEntry* entry = findFile(innerPath);
    if (entry != nullptr) {
        const std::filesystem::path extractedPath = std::filesystem::path("extracted") / (filename + ".cdr");
        extract(*entry, extractedPath);
        std::cout << "Extracted successfully to '" << extractedPath.string() << "'" << std::endl;
    }
    else
    {
        std::cerr << "File not found" << std::endl;
    }
}

void PAKParser::extract(const FileEntry& entry, const std::filesystem::path& outputPath) const {
    std::filesystem::create_directories(outputPath.parent_path());

    std::ifstream in(m_path.string(), std::ios::binary);
    std::vector<char> data(fileSize());
    in.read(data.data(), data.size());

    std::ofstream out(outputPath.string(), std::ios::binary);
    assert(out.is_open());
    out.write(data.data() + entry.offset, entry.size);
}

const FileEntry* PAKParser::findFile(std::string innerPath) const {
    std::transform(innerPath.begin(), innerPath.end(), innerPath.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return findFile(innerPath, "", 0);
}

const FileEntry* PAKParser::findFile(std::string innerPathLeft, std::string innerPathPassed, int offset) const {
    int num = char2hex(innerPathLeft[0]);
    if (num < 0)
        return nullptr;

    num += offset;
    if (num >= m_entries.size())
        return nullptr;

    std::string partial = innerPathLeft[0] + m_entries[num].partialName;
    if (!innerPathLeft._Starts_with(partial))
        return nullptr;

    innerPathPassed += partial;
    innerPathLeft = std::string(innerPathLeft.begin() + partial.size(), innerPathLeft.end());
    if (innerPathPassed.size() != m_entries[num].hLen + 1)
        return nullptr;

    if (m_entries[num].isRealFile())
    {
        if (innerPathLeft.size() != 0)
            return nullptr;
        return &m_entries[num];
    }
    if (innerPathLeft.size() < 1)
        return nullptr;
    return findFile(innerPathLeft, innerPathPassed, m_entries[num].hOffset);
}

void PAKParser::parse() {
    m_entries.clear();

    std::ifstream file(m_path.string(), std::ios::binary);
    std::vector<char> data(fileSize());
    file.read(data.data(), data.size());
    size_t pos = 0;

    // read magic
    const std::string magic = "tlj_pack0001";
    for (int i = 0; i < magic.size(); i++)
        if (data[i] != magic[i])
            std::cout << "tljpak id mismatch";
    pos += magic.size();

    uint32_t fileCount = readUint32(data.data(), pos);
    uint32_t numCount = readUint32(data.data(), pos);
    uint32_t byteCount = readUint32(data.data(), pos);

    //std::cout << fileCount << std::endl;
    //std::cout << numCount << std::endl;
    //std::cout << byteCount << std::endl;

    std::vector<char> nameBlock(byteCount);
    std::vector<int> lenBlock(numCount);

    for (uint32_t i = 0; i < fileCount; ++i)
        m_entries.emplace_back(data.data(), pos);

    std::vector<char> byteBlock(data.begin() + pos, data.begin() + pos + byteCount);
    pos += byteCount;

    for (uint32_t i = 0; i < byteCount; ++i)
        nameBlock[i] = hex2char((byteBlock[i]));

    for (uint32_t i = 0; i < numCount; ++i)
        lenBlock[i] = readInt32(data.data(), pos);

    for (auto& entry : m_entries)
        entry.fillIn(nameBlock);
}

size_t PAKParser::fileSize() const {
    std::ifstream file(m_path.string(), std::ios::binary);
    std::streampos begin = file.tellg();
    file.seekg (0, std::ios::end);
    std::streampos end = file.tellg();
    file.close();
    return end - begin;
}
