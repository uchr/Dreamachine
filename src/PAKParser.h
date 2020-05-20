#pragma once

#include "BinReader.h"

#include <filesystem>
#include <vector>
#include <unordered_map>

namespace parser
{

struct PAKFileEntry
{
    uint32_t offset, oldOffset;
    int32_t size;
    int32_t hOffset, hLen, hRef;
    std::string partialName = "";

    PAKFileEntry(BinReader& binReader);
    void fillIn(const std::vector<char>& nameBlock);
    bool isRealFile() const;
};

struct PAKIndex
{
    PAKIndex() = default;
    PAKIndex(const std::filesystem::path& path);

    std::vector<PAKFileEntry> entries;

    std::filesystem::path path;
};

class PAKParser
{
public:
    static PAKParser& instance(); // TODO: Remove

    PAKParser() = default;
    PAKParser(const std::filesystem::path& path);

    void tryExtract(const std::filesystem::path& innerPath);

private:
    void extract(const PAKIndex& pakIndex, const PAKFileEntry& entry, const std::filesystem::path& outputPath) const;

    PAKFileEntry* findFile(PAKIndex& pakIndex, std::string innerPath) const;
    PAKFileEntry* findFile(PAKIndex& pakIndex, std::string innerPathLeft, std::string innerPathPassed, int offset) const;

    std::unordered_map<std::string, PAKIndex> m_pakIndices;
};

}