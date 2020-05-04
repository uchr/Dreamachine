#include "SharkParser.h"
#include "BinReader.h"
#include "Utils.h"

#include <cassert>
#include <fstream>
#include <iostream>

std::vector<std::string> getSir(SharkNode* children)
{
    if (children == nullptr)
        return {};

    std::vector<std::string> sirs;
    for (int i = 0; i < children->count(); i++)
    {
        SharkNode* child = children->at(i);
        auto type = getEntryValue<std::string>(child, "type");
        if (type.has_value() && *type == "mod_engobj_funcom.loadtree")
        {
            auto name = getEntryValue<std::string>(child, "param/tree");
            if (name.has_value())
                sirs.emplace_back(*name);
        }

        if (type.has_value() && *type == "mod_core.capsule") {
            std::vector<std::string> newSirs = getSir(child->goSub("param/child_param/children"));
            sirs.reserve(sirs.size() + newSirs.size());
            std::move(std::begin(newSirs), std::end(newSirs), std::back_inserter(sirs));
        }
    }

    return sirs;
}

std::vector<std::string> getBpr(SharkNode* root)
{
    SharkNode* node = root->goSub("actor_param/child_param/children");
    for (int i = 0; node != nullptr && i < node->count(); i++) {
        auto type = getEntryValue<std::string>(node->at(i), "type");
        if (type.has_value() && *type == "mod_engobj_funcom.locationinit")
            return *getArray<std::string>(node->at(i), "param/bpr_files");
    }
    return {};
}

SharkParser::SharkParser(const std::filesystem::path& path) {
    BinReader binReader(path);
    if (binReader.readStringLine() != magic || binReader.readStringLine() != "2x4")
        throw std::exception("shark3d binary magic wrong");
    m_root = new SharkNodeValue(readSub(binReader), "root");
}

SharkParser::~SharkParser() {
    delete m_root;
}

SceneIndex SharkParser::parseScene() {
    std::ofstream treeOut("extracted/SceneTree.txt");
    print(m_root, treeOut, "");

    std::vector<std::string> sirs = getSir(m_root->goSub("actor_param/child_param/children"));
    std::vector<std::string> bprs = getBpr(m_root);

    SceneIndex sceneIndex;
    for (const auto& sirPath : sirs) {
        std::string filename = Utils::getFileNameWithoutExtension(sirPath);
        SirEntry entry{filename, sirPath};
        sceneIndex.sirs.emplace_back(entry);
    }

    return sceneIndex;
}

SharkNode* SharkParser::getRoot() const
{
    return m_root;
}

std::string SharkParser::indexString(BinReader& binReader)
{
    int num = static_cast<int>(binReader.readSharkNum());
    int index = m_stringCount - num;
    if (num == 0)
        m_stringCount++;
    if (m_stringTable.find(index) != m_stringTable.end())
        return m_stringTable[index];
    std::string result = binReader.readStringLine();
    m_stringTable.emplace(index, result);
    return result;
}

std::vector<SharkNode*> SharkParser::readSub(BinReader& binReader)
{
    int num = static_cast<int>(binReader.readSharkNum());

    std::vector<SharkNode*> nodes(num);
    for (int i = 0; i < num; i++)
    {
        std::string name = indexString(binReader);
        int attachCode = binReader.readByte();
        switch (attachCode)
        {
            case 0:
                nodes[i] = new SharkNode(name);
                break;
            case 1:
                nodes[i] = new SharkNodeValue(binReader.readSharkNum(), name);
                break;
            case 2:
                {
                    std::vector<int64_t> table(binReader.readSharkNum());
                    for (int e = 0; e < table.size(); e++)
                        table[e] = binReader.readSharkNum();
                    nodes[i] = new SharkNodeArray(table, name);
                    break;
                }
            case 4:
                nodes[i] = new SharkNodeValue(binReader.readEndianFloat(), name);
                break;
            case 8:
                {
                    std::vector<float> table(binReader.readSharkNum());
                    for (int e = 0; e < table.size(); e++)
                        table[e] = binReader.readEndianFloat();
                    nodes[i] = new SharkNodeArray(table, name);
                    break;
                }
            case 0x10:
                nodes[i] = new SharkNodeValue(indexString(binReader), name);
                break;
            case 0x20:
                {
                    std::vector<std::string> table(binReader.readSharkNum());
                    for (int e = 0; e < table.size(); e++)
                        table[e] = indexString(binReader);
                    nodes[i] = new SharkNodeArray(table, name);
                    break;
                }
            case 0x40:
                nodes[i] = new SharkNodeValue(readSub(binReader), name);
                break;
            case 0x80:
                {
                    std::vector<SharkNode*> table(binReader.readSharkNum());
                    for (int e = 0; e < table.size(); e++)
                        table[e] = new SharkNodeValue(readSub(binReader), name);
                    nodes[i] = new SharkNodeArray(table, name);
                    break;
                }
            default:
                std::cerr << "Unrecognized code in shark3d binary !" << std::endl;
                return {};
        }
    }
    return nodes;
}