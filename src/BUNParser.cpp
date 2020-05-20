#include "BUNParser.h"
#include "BinReader.h"
#include "PAKParser.h"

#include <spdlog/spdlog.h>

#include <array>
#include <cassert>

namespace parser
{

StreamFormat parseStreamFormat(BinReader& binReader) {
    StreamFormat streamFormat;
    streamFormat.size = binReader.read<int32_t>() / 4;
    for (int i = 0; i < 16; i++)
        streamFormat.channel[i] = binReader.read<int32_t>();
    streamFormat.streams = binReader.read<int32_t>() + 1;
    return streamFormat;
}

BUNParser::BUNParser(std::filesystem::path path)
    : m_path(std::move(path))
{
    PAKParser::instance().tryExtract(m_path);
}

BundleHeader BUNParser::parseHeader() {
    BinReaderMmap binReader(m_path);

    spdlog::debug("Parse bun header");
    int posOrigin = binReader.read<int32_t>() + 4;

    spdlog::debug("Reading textures");
    int numberOfTextures = binReader.read<int32_t>();
    std::vector<std::string> textures(numberOfTextures);
    for (int i = 0; i < numberOfTextures; ++i)
    {
        int len = binReader.readByte();
        textures[i] = binReader.readStringLine();
    }

    int numberOfDataHeaders = binReader.read<int32_t>();
    std::vector<VertexDataHeader> dataHeader(numberOfDataHeaders);
    for (int i = 0; i < numberOfDataHeaders; ++i)
    {
        dataHeader[i].vertexSize = binReader.read<int32_t>();
        dataHeader[i].length = binReader.read<int32_t>();
        dataHeader[i].posStart = binReader.getPosition();
        binReader.shiftPosition(dataHeader[i].length);
    }

    spdlog::debug("Reading 0pos");
    size_t posZero = binReader.getPosition();
    int numberOfFiles = binReader.read<int32_t>();
    int numStreamFormats = binReader.read<int32_t>();
    int unknown = binReader.read<int32_t>();

    std::vector<BundleFileEntry> fileEntries(numberOfFiles);
    for (int i = 0; i < numberOfFiles; ++i)
        fileEntries[i].posStart = binReader.read<uint32_t>();

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
        int numberOfMeshes = binReader.read<int32_t>();
        fileEntries[i].meshEntries.resize(numberOfMeshes);
        for (int j = 0; j < numberOfMeshes; ++j)
            fileEntries[i].meshEntries[j].posStart = binReader.read<uint32_t>();

        for (int j = 0; j < numberOfMeshes; ++j)
        {
            binReader.setPosition(fileEntries[i].meshEntries[j].posStart + posZero);
            binReader.setPosition(binReader.read<int32_t>() + posZero);
            fileEntries[i].meshEntries[j].name = binReader.readStringLine();
            fileEntries[i].meshEntries[j].dataIndex = dataIndex;
            binReader.setPosition(fileEntries[i].meshEntries[j].posStart + 0x54 + posZero);
            int numberParts = binReader.read<int32_t>();
            for (int k = 0; k < numberParts; ++k)
            {
                binReader.setPosition(fileEntries[i].meshEntries[j].posStart + 0x58 + k * 4 + posZero);
                binReader.setPosition(binReader.read<int32_t>() + 0x50 + posZero);
                int frmt = binReader.read<int32_t>();
                int formatIndex = (frmt / 4 - numberOfFiles - 3) / 18;
                int bitcode = binReader.read<int32_t>();
                int usage = binReader.read<int32_t>();
                if (bitcode != 0 && frmt != 0 && streamFormats[formatIndex].size != 0)
                {
                    binReader.shiftPosition(0x6c);
                    dataIndex += binReader.read<int32_t>();
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