#pragma once

#include "Utils.h"

#include <optional>
#include <string>
#include <vector>

namespace parser {

enum SharkNodeType
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

struct SharkNode;
SharkNode* getNode(SharkNode* parent, const std::string& name);
void print(SharkNode* node, std::ostream& out, const std::string& offset);

struct SharkNode {
    SharkNodeType type;
    std::string name;

    SharkNode(std::string name)
            : type(SharkNodeType::Empty)
            , name(std::move(name)) {}

    virtual ~SharkNode() = default;

    SharkNode* goSub(const std::string& path) {
        std::string next = Utils::splitString(path, '/')[0];
        SharkNode* nextNode = getNode(this, next);
        if (nextNode == nullptr || next == path)
            return nextNode;
        std::string leftPath = std::string(path.begin() + path.find('/') + 1, path.end());
        return nextNode->goSub(leftPath);
    }

    virtual SharkNode* at(int) { return nullptr; };

    virtual size_t count() { return 0; };
};

template <typename T>
struct SharkNodeValue : public SharkNode {
    T value;

    SharkNodeValue(T value, std::string name)
            : value(std::move(value))
            , SharkNode(name) {
        if constexpr (std::is_same_v<T, int64_t>)
            type = SharkNodeType::Int;
        else if constexpr (std::is_same_v<T, float>)
            type = SharkNodeType::Float;
        else if constexpr (std::is_same_v<T, std::string>)
            type = SharkNodeType::String;
        else
            static_assert(std::false_type::value, "unsupported type");
    }

    virtual ~SharkNodeValue() = default;
};

template <>
struct SharkNodeValue<std::vector<SharkNode*>> : public SharkNode {
    std::vector<SharkNode*> value;

    SharkNodeValue(std::vector<SharkNode*> value, std::string name)
            : value(std::move(value))
            , SharkNode(name) {
        type = SharkNodeType::Sub;
    }

    size_t count() override { return 1; };

    SharkNode* at(int) override { return dynamic_cast<SharkNode*>(this); }

    virtual ~SharkNodeValue() {
        for (auto* node : value) {
            delete node;
        }
    }
};

using SharkNodeSub = SharkNodeValue<std::vector<SharkNode*>>;

template <typename T>
struct SharkNodeArray : public SharkNode {
    std::vector<T> value;

    SharkNodeArray(std::vector<T> value, std::string name)
            : value(std::move(value))
            , SharkNode(name) {
        if constexpr (std::is_same_v<T, int64_t>)
            type = SharkNodeType::ArrayInt;
        else if constexpr (std::is_same_v<T, float>)
            type = SharkNodeType::ArrayFloat;
        else if constexpr (std::is_same_v<T, std::string>)
            type = SharkNodeType::ArrayString;
        else
            static_assert(std::false_type::value, "unsupported type");
    }

    size_t count() override { return value.size(); };

    virtual ~SharkNodeArray() = default;
};

template <>
struct SharkNodeArray<SharkNode*> : public SharkNode {
    std::vector<SharkNode*> value;

    SharkNodeArray(std::vector<SharkNode*> value, std::string name)
            : value(std::move(value))
            , SharkNode(name) {
        type = SharkNodeType::ArraySub;
    }

    size_t count() override { return value.size(); };

    SharkNode* at(int index) override { return value.at(index); }

    virtual ~SharkNodeArray() {
        for (auto* node : value) {
            delete node;
        }
    }
};

template <typename T>
std::optional<T> getEntryValue(SharkNode* node, const std::string& name) {
    SharkNode* cur = node->goSub(name);
    if (cur == nullptr)
        return std::nullopt;
    return dynamic_cast<SharkNodeValue<T>*>(cur)->value;
}

template <typename T>
std::optional<std::vector<T>> getEntryArray(SharkNode* node, const std::string& name) {
    SharkNode* cur = node->goSub(name);
    if (cur == nullptr)
        return std::nullopt;
    return dynamic_cast<SharkNodeArray<T>*>(cur)->value;
}

} // namespace parser