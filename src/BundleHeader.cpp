#include "BundleHeader.h"
#include "BinReader.h"

namespace parser
{

bool PartHeader::load(BinReader& binReader) {
    for (size_t i = 0; i < cfArray.size(); ++i)
        cfArray[i] = binReader.readUint32();
    numMagic = binReader.readInt32();
    posMagic = binReader.readUint32();
    formatIdx = binReader.readInt32();
    bitcode = binReader.readInt32();
    usage = binReader.readInt32();
    val5_3 = binReader.readInt32();
    val5_4 = binReader.readInt32();
    numIdx = binReader.readInt32();
    posIdx = binReader.readUint32();
    numBoneUsage = binReader.readInt32();
    posBoneUsage = binReader.readUint32();
    numBoneStages = binReader.readInt32();
    posBoneVerts = binReader.readUint32();
    posBoneIdx = binReader.readUint32();
    posBoneAssign = binReader.readUint32();
    lenXTable = binReader.readInt32();
    posXTable = binReader.readUint32();
    strange0 = binReader.readUint32();
    strange1 = binReader.readUint32();
    strange2 = binReader.readUint32();
    strange3 = binReader.readUint32();
    numTax1 = binReader.readInt32();
    posTax1 = binReader.readUint32();
    numTax2 = binReader.readInt32();
    posTax2 = binReader.readUint32();
    numTax3 = binReader.readInt32();
    posTax3 = binReader.readUint32();
    numTexStages = binReader.readInt32();
    posStageVerts = binReader.readUint32();
    posStageIdx = binReader.readUint32();
    posStageC = binReader.readUint32();
    posStageAssign = binReader.readUint32();
    numAnim = binReader.readInt32();
    posAnim = binReader.readUint32();
    numVertices = binReader.readInt32();
    for (size_t i = 0; i < posBonus.size(); ++i)
        posBonus[i] = binReader.readUint32();
    numIdxBonus = binReader.readInt32();
    posIdxBonus = binReader.readUint32();
    numTextures = binReader.readInt32();
    return true;
}

bool PartTex::load(BinReader& binReader, int numTexStages) {
    cf1 = binReader.readUint32(); cf2 = binReader.readUint32();
    posTex = binReader.readUint32(); unknown = binReader.readInt32();
    texIdx = binReader.readInt32Table(numTexStages, posTex);
    return true;
}

bool MeshPart::load(BinReader& binReader) {
    header.load(binReader);
    posTextures = binReader.readUint32Table(header.numTextures, 0);
    magic = binReader.readUint32Table(header.numMagic, header.posMagic);
    binReader.Assert(header.posIdx); indices = binReader.readChars(header.numIdx * 2);
    boneUsage = binReader.readUint16Table(header.numBoneUsage, header.posBoneUsage);
    boneVertices = binReader.readUint16Table(header.numBoneStages, header.posBoneVerts);
    boneIndices = binReader.readUint16Table(header.numBoneStages, header.posBoneIdx);
    boneAssign = (header.posBoneAssign == 0)
               ? std::nullopt
               : std::make_optional<std::vector<uint16_t>>(binReader.readUint16Table(header.numBoneStages, header.posBoneAssign));
    tax1 = binReader.readUint32Table(header.numTax1, header.posTax1);
    tax2 = binReader.readUint32Table(header.numTax2, header.posTax2);
    tax3 = binReader.readUint32Table(header.numTax3, header.posTax3);
    binReader.Assert(header.posXTable);
    xTable = binReader.readChars(header.lenXTable);
    stageVertices = binReader.readInt32Table(header.numTexStages, header.posStageVerts);
    stageIndices = binReader.readInt32Table(header.numTexStages, header.posStageIdx);
    stageC = (header.posStageC == 0) ? std::nullopt : std::make_optional<std::vector<int32_t>>(binReader.readInt32Table(header.numTexStages, header.posStageC));
    stageAssign = binReader.readInt32Table(header.numTexStages, header.posStageAssign);;
    animKeys = binReader.readFloatTable(header.numAnim, header.posAnim);
    if ((header.usage & 1) != 0) bonus1 = binReader.readUint32Table(3 * header.numVertices, header.posBonus[0]);
    if ((header.usage & 2) != 0) bonus2 = binReader.readUint32Table(3 * header.numVertices, header.posBonus[1]);
    if (header.numIdxBonus != 0 && binReader.IsPos(header.posIdxBonus)) idxBonus = binReader.readUint32Table( header.numIdxBonus, 0);
    tex.resize(header.numTextures);
    for (int i = 0; i < header.numTextures; ++i)
        tex[i].load(binReader, header.numTexStages);
    return !tex.empty();
}

bool MeshHeader::load(BinReader& binReader) {
    posName = binReader.readUint32();
    rescale = binReader.readFloat();
    posCenter.x = binReader.readFloat();
    posCenter.y = binReader.readFloat();
    posCenter.z = binReader.readFloat();
    posBound.x = binReader.readFloat();
    posBound.y = binReader.readFloat();
    posBound.z = binReader.readFloat();
    for (size_t i = 0; i < zero1.size(); ++i)
        zero1[i] = binReader.readUint32();
    numBones = binReader.readInt32();
    posBoneNames = binReader.readUint32();
    posBoneData = binReader.readUint32();
    numTextures = binReader.readInt32();
    posTextures = binReader.readUint32();
    zero2 = binReader.readUint32();
    zero3 = binReader.readUint32();
    numParts = binReader.readInt32();
    return true;
}

bool MeshInfo::load(BinReader& binReader) {
    header.load(binReader);
    if (header.numParts == 0)
        return false;

    posParts = binReader.readUint32Table(header.numParts, 0);
    binReader.Assert(header.posName);
    name = binReader.readStringLine();
    boneNames.resize(header.numBones);
    for (int k = 0; k < header.numBones; k++) {
        std::vector<char> boneNameBuffer = binReader.readChars(0x28);
        boneNames[k] = std::string(boneNameBuffer.begin(), std::find(boneNameBuffer.begin(), boneNameBuffer.end(), '\0'));
    }
    boneData = binReader.readFloatTable(7 * header.numBones, header.posBoneData);
    texIdx = binReader.readInt32Table(header.numTextures, header.posTextures);
    parts.resize(header.numParts);
    for (int i = 0; i < header.numParts; ++i) {
        parts[i].load(binReader);
    }
    return !parts.empty();
}

}