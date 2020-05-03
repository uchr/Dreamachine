#pragma once

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <optional>

inline std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
       tokens.push_back(token);
    }
    return tokens;
}

enum NodeType
{
    Empty = 0,
    Int,
    ArrayInt,
    Float,
    ArrayFloat,
    String,
    ArrayString,
    Sub,
    ArraySub
};

struct SceneNode;
SceneNode* getNode(SceneNode* parent, const std::string& name);

struct SceneNode
{
    NodeType type;
    std::string name;

    SceneNode(std::string name)
        : type(NodeType::Empty)
        , name(std::move(name))
    {
    }

    virtual ~SceneNode() = default;

    SceneNode* goSub(const std::string& path)
    {
        std::string next = split(path, '/')[0];
        SceneNode* nextNode = getNode(this, next);
        if (nextNode == nullptr || next == path)
            return nextNode;
        std::string leftPath = std::string(path.begin() + path.find('/') + 1, path.end());
        return nextNode->goSub(leftPath);
    }

    virtual SceneNode* at(int index) {
        return nullptr;
    };

    virtual size_t count() {
        return 0;
    };
};

template<typename T>
struct SceneNodeValue : public SceneNode {
    T value;

    SceneNodeValue(T value, std::string name)
        : value(std::move(value))
        , SceneNode(name)
    {
        if constexpr (std::is_same_v<T, int64_t>)
            type = NodeType::Int;
        else if constexpr (std::is_same_v<T, float>)
            type = NodeType::Float;
        else if constexpr (std::is_same_v<T, std::string>)
            type = NodeType::String;
        else
            static_assert(std::false_type::value, "unsupported type");
    }

    virtual ~SceneNodeValue() = default;
};

template<>
struct SceneNodeValue<std::vector<SceneNode*>> : public SceneNode {
    std::vector<SceneNode*> value;

    SceneNodeValue(std::vector<SceneNode*> value, std::string name)
        : value(std::move(value))
        , SceneNode(name)
    {
        type = NodeType::Sub;
    }

    size_t count() override {
        return 1;
    };

    SceneNode* at(int index) override
    {
        return dynamic_cast<SceneNode*>(this);
    }

    virtual ~SceneNodeValue() {
        for (auto* node : value) {
            delete node;
        }
    }
};

using SceneNodeSub = SceneNodeValue<std::vector<SceneNode*>>;

template<typename T>
struct SceneNodeArray : public SceneNode {
    std::vector<T> value;

    SceneNodeArray(std::vector<T> value, std::string name)
        : value(std::move(value))
        , SceneNode(name)
    {
        if constexpr (std::is_same_v<T, int64_t>)
            type = NodeType::ArrayInt;
        else if constexpr (std::is_same_v<T, float>)
            type = NodeType::ArrayFloat;
        else if constexpr (std::is_same_v<T, std::string>)
            type = NodeType::ArrayString;
        else
            static_assert(std::false_type::value, "unsupported type");
    }

    size_t count() override {
        return value.size();
    };

    virtual ~SceneNodeArray() = default;
};

template<>
struct SceneNodeArray<SceneNode*> : public SceneNode {
    std::vector<SceneNode*> value;

    SceneNodeArray(std::vector<SceneNode*> value, std::string name)
        : value(std::move(value))
        , SceneNode(name)
    {
        type = NodeType::ArraySub;
    }

    size_t count() override {
        return value.size();
    };

    SceneNode* at(int index) override
    {
        return value.at(index);
    }

    virtual ~SceneNodeArray() {
        for (auto* node : value) {
            delete node;
        }
    }
};

inline SceneNode* getNode(SceneNode* parent, const std::string& name)
{
    if (parent->type != NodeType::Sub)
        return nullptr;

    SceneNodeSub* subNode = dynamic_cast<SceneNodeSub*>(parent);
    for (SceneNode* node : subNode->value)
        if (node->name == name)
            return node;
    return nullptr;
}

template <typename T>
std::optional<T> getEntryValue(SceneNode* node, const std::string& name)
{
    SceneNode* cur = node->goSub(name);
    if (cur == nullptr)
        return std::nullopt;
    return dynamic_cast<SceneNodeValue<T>*>(cur)->value;
}

template <typename T>
std::optional<std::vector<T>> getArray(SceneNode* node, const std::string& name)
{
    SceneNode* cur = node->goSub(name);
    if (cur == nullptr)
        return std::nullopt;
    return dynamic_cast<SceneNodeArray<T>*>(cur)->value;
}

inline void print(SceneNode* root, std::ostream& out, const std::string& offset)
{
    out << offset << root->name;
    if (root->type == NodeType::Int)
        out << " = " << dynamic_cast<SceneNodeValue<int64_t>*>(root)->value;
    if (root->type == NodeType::Float)
        out << " = " << dynamic_cast<SceneNodeValue<float>*>(root)->value;
    if (root->type == NodeType::String)
        out << " = " << dynamic_cast<SceneNodeValue<std::string>*>(root)->value;
    if (root->type == NodeType::Sub)
        out << ": (s)";
    
    if (root->type == NodeType::ArrayInt)
        out << " = int[" << root->count() << "]";
    if (root->type == NodeType::ArrayFloat)
        out << " = float[" << root->count() << "]";
    if (root->type == NodeType::ArrayString)
        out << " = string[" << root->count() << "]";
    if (root->type == NodeType::ArraySub)
        out << ": [" << root->count() << "]";
    out << std::endl;

    SceneNodeSub* subNode = dynamic_cast<SceneNodeSub*>(root);
    if (subNode != nullptr) {
        for (SceneNode* node : subNode->value) {
            print(node, out, offset + " ");
        }
    }

    SceneNodeArray<SceneNode*>* subNodeArray = dynamic_cast<SceneNodeArray<SceneNode*>*>(root);
    if (subNodeArray != nullptr) {
        for (SceneNode* node : subNodeArray->value) {
            print(node, out, offset + " ");
        }
    }
}
