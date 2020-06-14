#pragma once

#include "SceneIndex.h"
#include "SharkNode.h"

#include <filesystem>
#include <vector>
#include <unordered_map>
#include <string>

namespace parser
{

class BinReader;

class SharkParser {
public:
    SharkParser(const std::filesystem::path& path);
    ~SharkParser();

    SceneIndex parseScene(const std::string& bundleName); // TODO: Move it outside SharkParser

    SharkNode* getRoot() const;

private:
    std::string indexString(BinReader& binReader);
    std::vector<SharkNode*> readSub(BinReader& binReader);

    int m_stringCount = 0;

    SharkNode* m_root;

    std::unordered_map<int, std::string> m_stringTable;

    const std::string magic = "shark3d_snake_binary";
};

}