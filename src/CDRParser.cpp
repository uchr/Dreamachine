#include "CDRParser.h"
#include "BinReader.h"
#include "SceneNode.h"

#include <cassert>
#include <fstream>
#include <iostream>

std::vector<std::string> getSir(SceneNode* children)
{
    if (children == nullptr)
        return {};

    std::vector<std::string> sirFilePathes;
    for (int i = 0; i < children->count(); i++)
    {
        SceneNode* child = children->at(i);
        auto type = getEntryValue<std::string>(child, "type");
        if (type.has_value() && *type == "mod_engobj_funcom.loadtree")
        {
            auto name = getEntryValue<std::string>(child, "param/tree");
            if (name.has_value())
                sirFilePathes.emplace_back(*name);
        }

        if (type.has_value() && *type == "mod_core.capsule") {
            std::vector<std::string> newSirFiles = getSir(child->goSub("param/child_param/children"));
            sirFilePathes.reserve(sirFilePathes.size() + newSirFiles.size());
            std::move(std::begin(newSirFiles), std::end(newSirFiles), std::back_inserter(sirFilePathes));
        }
    }

    return sirFilePathes;
}

std::vector<std::string> getBpr(SceneNode* root)
{
    SceneNode* node = root->goSub("actor_param/child_param/children");
    for (int i = 0; node != nullptr && i < node->count(); i++) {
        auto type = getEntryValue<std::string>(node->at(i), "type");
        if (type.has_value() && *type == "mod_engobj_funcom.locationinit")
            return *getArray<std::string>(node->at(i), "param/bpr_files");
    }
    return {};
}

CDRParser::CDRParser(std::filesystem::path path)
    : m_path(std::move(path))
{
    BinReader binReader(m_path);
    if (binReader.readStringLine() != magic || binReader.readStringLine() != "2x4")
        throw std::exception("shark3d binary magic wrong");
    m_root = new SceneNodeValue(readSub(binReader), "root");
}

void CDRParser::parseScene() {
    std::ofstream treeOut("extracted/SceneTree.txt");
    print(m_root, treeOut, "");

    std::vector<std::string> sirFilePathes = getSir(m_root->goSub("actor_param/child_param/children"));
    std::cout << ".sir:" << std::endl;
    for (const auto& filePath : sirFilePathes)
        std::cout << filePath << std::endl;

    //std::vector<std::string> bprFilePathes = getBpr(m_root);
    //std::cout << ".bpr:" << std::endl;
    //for (const auto& name : bprFilePathes)
    //    std::cout << name << std::endl;
}

std::string CDRParser::indexString(BinReader& binReader)
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

std::vector<SceneNode*> CDRParser::readSub(BinReader& binReader)
{
    int num = static_cast<int>(binReader.readSharkNum());

    std::vector<SceneNode*> nodes(num);
    for (int i = 0; i < num; i++)
    {
        std::string name = indexString(binReader);
        int attachCode = binReader.readByte();
        switch (attachCode)
        {
            case 0:
                nodes[i] = new SceneNode(name);
                break;
            case 1:
                nodes[i] = new SceneNodeValue(binReader.readSharkNum(), name);
                break;
            case 2:
                {
                    std::vector<int64_t> table(binReader.readSharkNum());
                    for (int e = 0; e < table.size(); e++)
                        table[e] = binReader.readSharkNum();
                    nodes[i] = new SceneNodeArray(table, name);
                    break;
                }
            case 4:
                nodes[i] = new SceneNodeValue(binReader.readEndianFloat(), name);
                break;
            case 8:
                {
                    std::vector<float> table(binReader.readSharkNum());
                    for (int e = 0; e < table.size(); e++)
                        table[e] = binReader.readEndianFloat();
                    nodes[i] = new SceneNodeArray(table, name);
                    break;
                }
            case 0x10:
                nodes[i] = new SceneNodeValue(indexString(binReader), name);
                break;
            case 0x20:
                {
                    std::vector<std::string> table(binReader.readSharkNum());
                    for (int e = 0; e < table.size(); e++)
                        table[e] = indexString(binReader);
                    nodes[i] = new SceneNodeArray(table, name);
                    break;
                }
            case 0x40:
                nodes[i] = new SceneNodeValue(readSub(binReader), name);
                break;
            case 0x80:
                {
                    std::vector<SceneNode*> table(binReader.readSharkNum());
                    for (int e = 0; e < table.size(); e++)
                        table[e] = new SceneNodeValue(readSub(binReader), name);
                    nodes[i] = new SceneNodeArray(table, name);
                    break;
                }
            default:
                std::cerr << "Unrecognized code in shark3d binary !" << std::endl;
                return {};
        }
    }
    return nodes;
}