#pragma once

#include "GeometryData.h"

#include <vector>
#include <string>
#include <array>
#include <optional>

namespace parser
{

class BinReader;

struct PartHeader
{
    std::array<uint32_t, 18> cfArray;
    int32_t numMagic;
    uint32_t posMagic;
    int32_t formatIdx;
    int32_t bitcode;
    int32_t usage;
    int32_t val5_3;
    int32_t val5_4;
    int32_t numIdx;
    uint32_t posIdx;
    int32_t numBoneUsage;
    uint32_t posBoneUsage;
    int32_t numBoneStages;
    uint32_t posBoneVerts;
    uint32_t posBoneIdx;
    uint32_t posBoneAssign;
    int32_t lenXTable;
    uint32_t posXTable;
    uint32_t strange0;
    uint32_t strange1;
    uint32_t strange2;
    uint32_t strange3;
    int32_t numTax1;
    uint32_t posTax1;
    int32_t numTax2;
    uint32_t posTax2;
    int32_t numTax3;
    uint32_t posTax3;
    int32_t numTexStages;
    uint32_t posStageVerts;
    uint32_t posStageIdx;
    uint32_t posStageC;
    uint32_t posStageAssign;
    int32_t numAnim;
    uint32_t posAnim;
    int32_t numVertices;
    std::array<uint32_t, 10> posBonus;
    int32_t numIdxBonus;
    uint32_t posIdxBonus;
    int32_t numTextures;

    bool load(BinReader& binReader);
};

struct PartTexInfo
{
    uint32_t cf1, cf2;
    uint32_t posTex;
    int32_t unknown;
    std::vector<int32_t> texIdx;

    bool load(BinReader& binReader, int numTexStages);
};

struct MeshPartInfo
{
    PartHeader header;
    std::vector<uint32_t> posTextures;
    std::vector<uint32_t> magic;
    std::vector<char> indices;
    std::vector<uint16_t> boneUsage;
    std::vector<uint16_t> boneVertices, boneIndices;
    std::optional<std::vector<uint16_t>> boneAssign;
    std::vector<uint32_t> tax1, tax2, tax3;
    std::vector<char> xTable;
    std::vector<int32_t> stageVertices, stageIndices, stageAssign;
    std::optional<std::vector<int32_t>> stageC;
    std::vector<float> animKeys;
    std::vector<uint32_t> bonus1, bonus2;
    std::vector<uint32_t> idxBonus;
    std::vector<PartTexInfo> tex;

    bool load(BinReader& binReader);
};

struct MeshHeader
{
    uint32_t posName;
    float rescale;
    Vector3 posCenter;
    Vector3 posBound;
    std::array<uint32_t, 6> zero1;
    int32_t numBones;
    uint32_t posBoneNames;
    uint32_t posBoneData;
    int32_t numTextures;
    uint32_t posTextures;
    uint32_t zero2;
    uint32_t zero3;
    int32_t numParts;

    bool load(BinReader& binReader);
};

struct MeshInfo
{
    MeshHeader header;
    std::string name;
    std::vector<uint32_t> posParts;
    std::vector<std::string> boneNames;
    std::vector<float> boneData;
    std::vector<int32_t> texIdx;
    std::vector<MeshPartInfo> parts;

    bool load(BinReader& binReader);
};

struct MeshEntry
{
    size_t posStart;
    std::string name;
    int dataIndex;
    MeshInfo mesh;
};

struct BundleFileEntry
{
    size_t posStart;
    std::string smrName;
    std::vector<MeshEntry> meshEntries;

    MeshEntry* getMeshEntry(const std::string& name) // TODO: Refactor this
    {
        for (MeshEntry& file : meshEntries) {
            if (file.name == name)
                return &file;
        }
        return nullptr;
    }
};

struct VertexDataHeader
{
    size_t posStart;
    int vertexSize, length;
    std::vector<char> data;
};

struct StreamFormat
{
    int32_t size;
    std::array<int, 16> channel;
    int32_t streams;
};

struct BundleHeader  {
    size_t posZero, posOrigin;
    std::vector<std::string> textures;
    std::vector<VertexDataHeader> dataHeader;
    std::vector<StreamFormat> streamFormats;
    std::vector<BundleFileEntry> fileEntries;

    BundleFileEntry* getFileEntry(const std::string& name) // TODO: Refactor this
    {
        for (BundleFileEntry& file : fileEntries) {
            if (file.smrName == name)
                return &file;
        }
        return nullptr;
    }
};

}