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

PAKFileEntry::PAKFileEntry(BinReader& binReader) {
    offset = binReader.readUint32();
    size = binReader.readInt32();
    hOffset = binReader.readInt32();
    hLen = binReader.readInt32();
    hRef = binReader.readInt32();
    if (isRealFile())
        --hLen;
}

void PAKFileEntry::fillIn(const std::vector<char>& nameBlock)
{
    for (int i = hRef; i < nameBlock.size(); i++)
    {
        if (nameBlock[i] == 0)
            break;
        partialName += nameBlock[i];
    }
}

bool PAKFileEntry::isRealFile() const
{
    return size > 0;
}

PAKParser& PAKParser::instance() {
    static PAKParser pakParser;
    return pakParser;
}

PAKParser::PAKParser() = default;

PAKParser::PAKParser(std::filesystem::path path) 
    : m_binReader(path)
{
    parse();
}

PAKParser::~PAKParser() = default;

void PAKParser::tryExtract(const std::filesystem::path& path) {
    std::string innerPath = path.string();
    std::replace(innerPath.begin(), innerPath.end(), '/', '\\');
    std::cout << "Try to extract " << innerPath << std::endl;

    const PAKFileEntry* entry = findFile(innerPath);
    if (entry != nullptr) {
        extract(*entry, path);
        std::cout << "Extracted successfully to " << path << std::endl;
    }
    else
    {
        std::cerr << "File not found" << std::endl;
    }
}

void PAKParser::extract(const PAKFileEntry& entry, const std::filesystem::path& outputPath) const {
    std::filesystem::create_directories(outputPath.parent_path());
    std::ofstream out(outputPath.string(), std::ios::binary);
    assert(out.is_open());

    out.write(m_binReader.data() + entry.offset, entry.size);
}

const PAKFileEntry* PAKParser::findFile(std::string innerPath) const {
    std::transform(innerPath.begin(), innerPath.end(), innerPath.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return findFile(innerPath, "", 0);
}

const PAKFileEntry* PAKParser::findFile(std::string innerPathLeft, std::string innerPathPassed, int offset) const {
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

    m_binReader.setPosition(0);

    // read magic
    const std::string magicRequirement = "tlj_pack0001";
    const std::string magic = m_binReader.readString(magicRequirement.size());
    if (magicRequirement != magic)
        throw std::runtime_error("tljpak magic start is missing");

    uint32_t fileCount = m_binReader.readUint32();
    uint32_t numCount = m_binReader.readUint32();
    uint32_t byteCount = m_binReader.readUint32();

    std::vector<char> nameBlock(byteCount);
    std::vector<int> lenBlock(numCount);

    for (uint32_t i = 0; i < fileCount; ++i)
        m_entries.emplace_back(m_binReader);

    std::vector<char> byteBlock = m_binReader.readChars(byteCount);
    for (uint32_t i = 0; i < byteCount; ++i)
        nameBlock[i] = hex2char((byteBlock[i]));

    for (uint32_t i = 0; i < numCount; ++i)
        lenBlock[i] = m_binReader.readInt32();

    for (auto& entry : m_entries)
        entry.fillIn(nameBlock);
}
