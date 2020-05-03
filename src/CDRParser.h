#pragma once

#include "SceneIndex.h"
#include "SceneNode.h"

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <string>

class BinReader;

class CDRParser {
public:
    CDRParser(const std::filesystem::path& path);
    ~CDRParser();

    SceneIndex parseScene();

    SceneNode* getRoot() const;

private:
    std::string indexString(BinReader& binReader);
    std::vector<SceneNode*> readSub(BinReader& binReader);

    int m_stringCount = 0;

    SceneNode* m_root;

    std::unordered_map<int, std::string> m_stringTable;

    const std::string magic = "shark3d_snake_binary";
};
