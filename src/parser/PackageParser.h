#pragma once

#include "BinReader.h"

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace parser {

struct PackageFileEntry {
    uint32_t offset;
    int32_t size;
    int32_t hOffset, hLen, hRef;
    std::string partialName = "";

    PackageFileEntry(BinReader& binReader);
    void fillIn(const std::vector<char>& nameBlock);
    bool isRealFile() const;
};

struct PackageIndex {
    PackageIndex() = default;
    PackageIndex(const std::filesystem::path& path);

    std::vector<PackageFileEntry> entries;

    std::filesystem::path path;
};

// For parsing .pak files
class PackageParser {
public:
    static PackageParser& instance(); // TODO: Remove

    PackageParser() = default;
    PackageParser(const std::filesystem::path& path);

    void tryExtract(const std::filesystem::path& innerPath);

private:
    void extract(const PackageIndex& pakIndex, const PackageFileEntry& entry, const std::filesystem::path& outputPath) const;

    PackageFileEntry* findFile(PackageIndex& pakIndex, std::string innerPath) const;
    PackageFileEntry* findFile(PackageIndex& pakIndex, std::string innerPathLeft, std::string innerPathPassed, int offset) const;

    std::unordered_map<std::string, PackageIndex> m_pakIndices;

    std::unordered_set<std::string> m_extracted;
};

} // namespace parser