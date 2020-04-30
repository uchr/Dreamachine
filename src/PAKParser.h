#pragma once

#include <filesystem>
#include <vector>

struct FileEntry;

class PAKParser
{
public:
    PAKParser(std::filesystem::path path);
    ~PAKParser();

    void extractCDR() const;

private:
    void extract(const FileEntry& entry, const std::filesystem::path& outputPath) const;

    const FileEntry* findFile(std::string innerPath) const;
    const FileEntry* findFile(std::string innerPathLeft, std::string innerPathPassed, int offset) const;

    void parse();
    size_t fileSize() const;

    std::filesystem::path m_path;
    std::vector<FileEntry> m_entries;
};
