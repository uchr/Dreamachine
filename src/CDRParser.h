#pragma once

#include "SceneIndex.h"

#include <filesystem>
#include <unordered_map>
#include <string>

class BinReader;
struct SceneNode;

class CDRParser {
public:
    CDRParser(const std::filesystem::path& path);
    ~CDRParser();

    SceneIndex parseScene();

private:
    std::string indexString(BinReader& binReader);
    std::vector<SceneNode*> readSub(BinReader& binReader);

    int m_stringCount = 0;

    std::unordered_map<int, std::string> m_stringTable;
    SceneNode* m_root;

    const std::string magic = "shark3d_snake_binary";
};
