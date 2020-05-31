#include "BundleHeader.h"
#include "BinReader.h"

namespace parser
{

bool PartHeader::load(BinReader& binReader) {
    for (size_t i = 0; i < cfArray.size(); ++i)
        cfArray[i] = binReader.read<uint32_t>();
    numMagic = binReader.read<int32_t>();
    posMagic = binReader.read<uint32_t>();
    formatIdx = binReader.read<int32_t>();
    bitcode = binReader.read<int32_t>();
    usage = binReader.read<int32_t>();
    val5_3 = binReader.read<int32_t>();
    val5_4 = binReader.read<int32_t>();
    numIdx = binReader.read<int32_t>();
    posIdx = binReader.read<uint32_t>();
    numBoneUsage = binReader.read<int32_t>();
    posBoneUsage = binReader.read<uint32_t>();
    numBoneStages = binReader.read<int32_t>();
    posBoneVerts = binReader.read<uint32_t>();
    posBoneIdx = binReader.read<uint32_t>();
    posBoneAssign = binReader.read<uint32_t>();
    lenXTable = binReader.read<int32_t>();
    posXTable = binReader.read<uint32_t>();
    strange0 = binReader.read<uint32_t>();
    strange1 = binReader.read<uint32_t>();
    strange2 = binReader.read<uint32_t>();
    strange3 = binReader.read<uint32_t>();
    numTax1 = binReader.read<int32_t>();
    posTax1 = binReader.read<uint32_t>();
    numTax2 = binReader.read<int32_t>();
    posTax2 = binReader.read<uint32_t>();
    numTax3 = binReader.read<int32_t>();
    posTax3 = binReader.read<uint32_t>();
    numTexStages = binReader.read<int32_t>();
    posStageVerts = binReader.read<uint32_t>();
    posStageIdx = binReader.read<uint32_t>();
    posStageC = binReader.read<uint32_t>();
    posStageAssign = binReader.read<uint32_t>();
    numAnim = binReader.read<int32_t>();
    posAnim = binReader.read<uint32_t>();
    numVertices = binReader.read<int32_t>();
    for (size_t i = 0; i < posBonus.size(); ++i)
        posBonus[i] = binReader.read<uint32_t>();
    numIdxBonus = binReader.read<int32_t>();
    posIdxBonus = binReader.read<uint32_t>();
    numTextures = binReader.read<int32_t>();
    return true;
}

bool PartTexInfo::load(BinReader& binReader, int numTexStages) {
    cf1 = binReader.read<uint32_t>(); cf2 = binReader.read<uint32_t>();
    posTex = binReader.read<uint32_t>(); unknown = binReader.read<int32_t>();
    texIdx = binReader.readTable<int32_t>(numTexStages, posTex);
    return true;
}

bool MeshPartInfo::load(BinReader& binReader) {
    header.load(binReader);
    posTextures = binReader.readTable<uint32_t>(header.numTextures, 0);
    magic = binReader.readTable<uint32_t>(header.numMagic, header.posMagic);
    binReader.Assert(header.posIdx); indices = binReader.readChars(header.numIdx * 2);
    boneUsage = binReader.readTable<uint16_t>(header.numBoneUsage, header.posBoneUsage);
    boneVertices = binReader.readTable<uint16_t>(header.numBoneStages, header.posBoneVerts);
    boneIndices = binReader.readTable<uint16_t>(header.numBoneStages, header.posBoneIdx);
    boneAssign = (header.posBoneAssign == 0)
               ? std::nullopt
               : std::make_optional<std::vector<uint16_t>>(binReader.readTable<uint16_t>(header.numBoneStages, header.posBoneAssign));
    tax1 = binReader.readTable<uint32_t>(header.numTax1, header.posTax1);
    tax2 = binReader.readTable<uint32_t>(header.numTax2, header.posTax2);
    tax3 = binReader.readTable<uint32_t>(header.numTax3, header.posTax3);
    binReader.Assert(header.posXTable);
    xTable = binReader.readChars(header.lenXTable);
    stageVertices = binReader.readTable<int32_t>(header.numTexStages, header.posStageVerts);
    stageIndices = binReader.readTable<int32_t>(header.numTexStages, header.posStageIdx);
    stageC = (header.posStageC == 0) ? std::nullopt : std::make_optional<std::vector<int32_t>>(binReader.readTable<int32_t>(header.numTexStages, header.posStageC));
    stageAssign = binReader.readTable<int32_t>(header.numTexStages, header.posStageAssign);;
    animKeys = binReader.readTable<float>(header.numAnim, header.posAnim);
    if ((header.usage & 1) != 0) bonus1 = binReader.readTable<uint32_t>(3 * header.numVertices, header.posBonus[0]);
    if ((header.usage & 2) != 0) bonus2 = binReader.readTable<uint32_t>(3 * header.numVertices, header.posBonus[1]);
    if (header.numIdxBonus != 0 && binReader.IsPos(header.posIdxBonus)) idxBonus = binReader.readTable<uint32_t>( header.numIdxBonus, 0);
    tex.resize(header.numTextures);
    for (int i = 0; i < header.numTextures; ++i)
        tex[i].load(binReader, header.numTexStages);
    return !tex.empty();
}

bool MeshHeader::load(BinReader& binReader) {
    posName = binReader.read<uint32_t>();
    rescale = binReader.read<float>();
    posCenter.x = binReader.read<float>();
    posCenter.y = binReader.read<float>();
    posCenter.z = binReader.read<float>();
    posBound.x = binReader.read<float>();
    posBound.y = binReader.read<float>();
    posBound.z = binReader.read<float>();
    for (size_t i = 0; i < zero1.size(); ++i)
        zero1[i] = binReader.read<uint32_t>();
    numBones = binReader.read<int32_t>();
    posBoneNames = binReader.read<uint32_t>();
    posBoneData = binReader.read<uint32_t>();
    numTextures = binReader.read<int32_t>();
    posTextures = binReader.read<uint32_t>();
    zero2 = binReader.read<uint32_t>();
    zero3 = binReader.read<uint32_t>();
    numParts = binReader.read<int32_t>();
    return true;
}

bool MeshInfo::load(BinReader& binReader) {
    header.load(binReader);
    if (header.numParts == 0)
        return false;

    posParts = binReader.readTable<uint32_t>(header.numParts, 0);
    binReader.Assert(header.posName);
    name = binReader.readStringLine();
    boneNames.resize(header.numBones);
    for (int k = 0; k < header.numBones; k++) {
        std::vector<char> boneNameBuffer = binReader.readChars(0x28);
        boneNames[k] = std::string(boneNameBuffer.begin(), std::find(boneNameBuffer.begin(), boneNameBuffer.end(), '\0'));
    }
    boneData = binReader.readTable<float>(7 * header.numBones, header.posBoneData);
    texIdx = binReader.readTable<int32_t>(header.numTextures, header.posTextures);
    parts.resize(header.numParts);
    for (int i = 0; i < header.numParts; ++i) {
        parts[i].load(binReader);
    }
    return !parts.empty();
}

}