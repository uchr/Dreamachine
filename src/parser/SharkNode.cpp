#include "SharkNode.h"

#include <sstream>

namespace parser
{

SharkNode* getNode(SharkNode* parent, const std::string& name)
{
    if (parent->type != SharkNodeType::Sub)
        return nullptr;

    SharkNodeSub* subNode = dynamic_cast<SharkNodeSub*>(parent);
    for (SharkNode* node : subNode->value)
        if (node->name == name)
            return node;
    return nullptr;
}

void print(SharkNode* node, std::ostream& out, const std::string& offset)
{
    out << offset << node->name;
    if (node->type == SharkNodeType::Int)
        out << " = " << dynamic_cast<SharkNodeValue<int64_t>*>(node)->value;
    if (node->type == SharkNodeType::Float)
        out << " = " << dynamic_cast<SharkNodeValue<float>*>(node)->value;
    if (node->type == SharkNodeType::String)
        out << " = " << dynamic_cast<SharkNodeValue<std::string>*>(node)->value;
    if (node->type == SharkNodeType::Sub)
        out << ": (s)";
    
    if (node->type == SharkNodeType::ArrayInt)
        out << " = int[" << node->count() << "]";
    if (node->type == SharkNodeType::ArrayFloat)
        out << " = float[" << node->count() << "]";
    if (node->type == SharkNodeType::ArrayString)
        out << " = string[" << node->count() << "]";
    if (node->type == SharkNodeType::ArraySub)
        out << ": [" << node->count() << "]";
    out << std::endl;

    SharkNodeSub* subNode = dynamic_cast<SharkNodeSub*>(node);
    if (subNode != nullptr) {
        for (SharkNode* child : subNode->value) {
            print(child, out, offset + " ");
        }
    }

    SharkNodeArray<SharkNode*>* subNodeArray = dynamic_cast<SharkNodeArray<SharkNode*>*>(node);
    if (subNodeArray != nullptr) {
        for (SharkNode* child : subNodeArray->value) {
            print(child, out, offset + " ");
        }
    }
}

}