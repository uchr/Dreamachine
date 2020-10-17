#include "PackageParser.h"
#include "Utils.h"

#include <spdlog/spdlog.h>

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

namespace {

const std::vector<char> charTable{'\0', 'a', 'b',  'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',  'm', 'n',
                                  'o',  'p', 'q',  'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\\', '?', '?',
                                  '-',  '_', '\'', '.', '0', '1', '2', '3', '4', '5', '6', '7', '8',  '9'};

char hex2char(int a) {
    if (a > charTable.size())
        return '?';
    return charTable[a];
}

int char2hex(char c) {
    auto it = std::find(charTable.begin(), charTable.end(), c);
    if (it == charTable.end())
        return -1;
    return static_cast<int>(std::distance(charTable.begin(), it));
}

} // namespace

namespace parser {

PackageFileEntry::PackageFileEntry(BinReader& binReader) {
    offset = binReader.read<uint32_t>();
    size = binReader.read<int32_t>();
    hOffset = binReader.read<int32_t>();
    hLen = binReader.read<int32_t>();
    hRef = binReader.read<int32_t>();
    if (isRealFile())
        --hLen;
}

void PackageFileEntry::fillIn(const std::vector<char>& nameBlock) {
    for (int i = hRef; i < nameBlock.size(); i++) {
        if (nameBlock[i] == 0)
            break;
        partialName += nameBlock[i];
    }
}

bool PackageFileEntry::isRealFile() const {
    return size > 0;
}

PackageIndex::PackageIndex(const std::filesystem::path& path)
        : path(path) {
    BinReaderMmap binReader(path);

    // read magic
    const std::string magicRequirement = "tlj_pack0001";
    const std::string magic = binReader.readString(magicRequirement.size());
    if (magicRequirement != magic)
        throw std::runtime_error("tljpak magic start is missing");

    uint32_t fileCount = binReader.read<uint32_t>();
    uint32_t numCount = binReader.read<uint32_t>();
    uint32_t byteCount = binReader.read<uint32_t>();

    std::vector<char> nameBlock(byteCount);
    std::vector<int> lenBlock(numCount);

    for (uint32_t i = 0; i < fileCount; ++i)
        entries.emplace_back(binReader);

    std::vector<char> byteBlock = binReader.readChars(byteCount);
    for (uint32_t i = 0; i < byteCount; ++i)
        nameBlock[i] = hex2char((byteBlock[i]));

    for (uint32_t i = 0; i < numCount; ++i)
        lenBlock[i] = binReader.read<int32_t>();

    for (auto& entry : entries)
        entry.fillIn(nameBlock);
}

PackageParser& PackageParser::instance() {
    static PackageParser packageParser;
    return packageParser;
}

PackageParser::PackageParser(const std::filesystem::path& path) {
    for (auto& childIt : std::filesystem::directory_iterator(path)) {
        auto childPath = childIt.path();
        if (childPath.extension().string() == ".pak")
            m_pakIndices[Utils::getFilenameWithoutExtension(childPath)] = PackageIndex(childPath);
    }
}

void PackageParser::tryExtract(const std::filesystem::path& path) {
    if (m_extracted.contains(path.string()))
        return;

    std::string innerPath = path.string();
    std::replace(innerPath.begin(), innerPath.end(), '/', '\\');
    spdlog::debug("Try to extract {}", innerPath);

    PackageIndex* pakIndex = nullptr;
    PackageFileEntry* entry = nullptr;
    for (auto& it : m_pakIndices) {
        entry = findFile(it.second, innerPath);
        if (entry != nullptr) {
            pakIndex = &it.second;
            break;
        }
    }

    if (pakIndex != nullptr && entry != nullptr) {
        extract(*pakIndex, *entry, path);
        m_extracted.insert(path.string());
        spdlog::debug("Extracted successfully to {}", path.string());
    }
    else {
        spdlog::warn("{} not found", innerPath);
    }
}

void PackageParser::extract(const PackageIndex& pakIndex, const PackageFileEntry& entry, const std::filesystem::path& outputPath) const {
    std::filesystem::create_directories(outputPath.parent_path());
    std::ofstream out(outputPath.string(), std::ios::binary);
    assert(out.is_open());

    BinReaderMmap binReader(pakIndex.path, entry.offset, entry.size);
    out.write(binReader.data(), entry.size);
}

PackageFileEntry* PackageParser::findFile(PackageIndex& pakIndex, std::string innerPath) const {
    std::transform(innerPath.begin(), innerPath.end(), innerPath.begin(), [](unsigned char c) { return std::tolower(c); });

    return findFile(pakIndex, innerPath, "", 0);
}

PackageFileEntry* PackageParser::findFile(PackageIndex& pakIndex, std::string innerPathLeft, std::string innerPathPassed, int offset) const {
    int num = char2hex(innerPathLeft[0]);
    if (num < 0)
        return nullptr;

    num += offset;
    if (num >= pakIndex.entries.size())
        return nullptr;

    std::string partial = innerPathLeft[0] + pakIndex.entries[num].partialName;
    if (!innerPathLeft._Starts_with(partial))
        return nullptr;

    innerPathPassed += partial;
    innerPathLeft = std::string(innerPathLeft.begin() + partial.size(), innerPathLeft.end());
    if (innerPathPassed.size() != pakIndex.entries[num].hLen + 1)
        return nullptr;

    if (pakIndex.entries[num].isRealFile()) {
        if (innerPathLeft.size() != 0)
            return nullptr;
        return &pakIndex.entries[num];
    }
    if (innerPathLeft.size() < 1)
        return nullptr;
    return findFile(pakIndex, innerPathLeft, innerPathPassed, pakIndex.entries[num].hOffset);
}

} // namespace parser