#include "BUNParser.h"
#include "BinReader.h"

#include <spdlog/spdlog.h>

#include <array>
#include <cassert>

namespace parser
{

StreamFormat parseStreamFormat(BinReader& binReader) {
    StreamFormat streamFormat;
    streamFormat.size = binReader.readInt32() / 4;
    for (int i = 0; i < 16; i++)
        streamFormat.channel[i] = binReader.readInt32();
    streamFormat.streams = binReader.readInt32() + 1;
    return streamFormat;
}

BUNParser::BUNParser(std::filesystem::path path)
    : m_path(std::move(path))
{
}

BundleHeader BUNParser::parseHeader() {
    BinReader binReader(m_path);

    spdlog::debug("Parse bun header");
    int posOrigin = binReader.readInt32() + 4;

    spdlog::debug("Reading textures");
    int numberOfTextures = binReader.readInt32();
    std::vector<std::string> textures(numberOfTextures);
    for (int i = 0; i < numberOfTextures; ++i)
    {
        int len = binReader.readByte();
        textures[i] = binReader.readStringLine();
    }

    int numberOfDataHeaders = binReader.readInt32();
    std::vector<VertexDataHeader> dataHeader(numberOfDataHeaders);
    for (int i = 0; i < numberOfDataHeaders; ++i)
    {
        dataHeader[i].vertexSize = binReader.readInt32();
        dataHeader[i].length = binReader.readInt32();
        dataHeader[i].posStart = binReader.getPosition();
        binReader.shiftPosition(dataHeader[i].length);
    }

    spdlog::debug("Reading 0pos");
    size_t posZero = binReader.getPosition();
    int numberOfFiles = binReader.readInt32();
    int numStreamFormats = binReader.readInt32();
    int unknown = binReader.readInt32();

    std::vector<BundleFileEntry> fileEntries(numberOfFiles);
    for (int i = 0; i < numberOfFiles; ++i)
        fileEntries[i].posStart = binReader.readUint32();

    spdlog::debug("Reading stream formats");
    std::vector<StreamFormat> streamFormats(numStreamFormats);
    for (int i = 0; i < numStreamFormats; ++i)
        streamFormats[i] = parseStreamFormat(binReader);

    int dataIndex = 0;
    for (int i = 0; i < numberOfFiles; ++i)
    {
        binReader.setPosition(fileEntries[i].posStart + posZero);
        std::vector<char> smrNameBuffer = binReader.readChars(0x80);
        fileEntries[i].smrName = std::string(smrNameBuffer.begin(), std::find(smrNameBuffer.begin(), smrNameBuffer.end(), '\0'));
        int numberOfMeshes = binReader.readInt32();
        fileEntries[i].meshEntries.resize(numberOfMeshes);
        for (int j = 0; j < numberOfMeshes; ++j)
            fileEntries[i].meshEntries[j].posStart = binReader.readUint32();

        for (int j = 0; j < numberOfMeshes; ++j)
        {
            binReader.setPosition(fileEntries[i].meshEntries[j].posStart + posZero);
            binReader.setPosition(binReader.readInt32() + posZero);
            fileEntries[i].meshEntries[j].name = binReader.readStringLine();
            fileEntries[i].meshEntries[j].dataIndex = dataIndex;
            binReader.setPosition(fileEntries[i].meshEntries[j].posStart + 0x54 + posZero);
            int numberParts = binReader.readInt32();
            for (int k = 0; k < numberParts; ++k)
            {
                binReader.setPosition(fileEntries[i].meshEntries[j].posStart + 0x58 + k * 4 + posZero);
                binReader.setPosition(binReader.readInt32() + 0x50 + posZero);
                int frmt = binReader.readInt32();
                int formatIndex = (frmt / 4 - numberOfFiles - 3) / 18;
                int bitcode = binReader.readInt32();
                int usage = binReader.readInt32();
                if (bitcode != 0 && frmt != 0 && streamFormats[formatIndex].size != 0)
                {
                    binReader.shiftPosition(0x6c);
                    dataIndex += binReader.readInt32();
                }
            }
        }
    }

    BundleHeader bundleHeader;
    bundleHeader.posZero = posZero;
    bundleHeader.posOrigin = posOrigin;
    bundleHeader.dataHeader = dataHeader;
    bundleHeader.streamFormats = streamFormats;
    bundleHeader.fileEntries = fileEntries;
    bundleHeader.textures = textures;
    return bundleHeader;
}

}