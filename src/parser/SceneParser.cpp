#include "SceneParser.h"

#include "BundleParser.h"
#include "BinReader.h"
#include "CommonPath.h"
#include "Mesh.h"
#include "PAKParser.h"
#include "SceneNode.h"
#include "SharkNode.h"
#include "SharkParser.h"
#include "TextureParser.h"

#define STBI_ONLY_PNG
#include <stb_image.h>

#include <spdlog/spdlog.h>

#include <array>

namespace parser {

const enum class ChannelType
{
    Unused,
    Float2,
    Float3,
    Float4,
    Color
};
const size_t entrySize[] = {0, 8, 12, 16, 4};
const std::string entryName[] = {"Unused", "Float2", "Float3", "Float4", "Color"};
const ChannelType entryType[] = {ChannelType::Unused, ChannelType::Float2, ChannelType::Float3, ChannelType::Float4, ChannelType::Color};

std::optional<Mesh>
parseMesh(const StreamFormat& streamFormat, const std::vector<char>& verticesBuffer, const std::vector<char>& indicesBuffer) {
    if (verticesBuffer.empty() || indicesBuffer.empty())
        return std::nullopt;

    std::vector<ChannelType> channelTypes;
    int bytePerVertex = 0;
    for (int i = 0; i < 16; ++i) {
        if (streamFormat.channel[i] != -1) {
            bytePerVertex += entrySize[streamFormat.channel[i]];
            channelTypes.push_back(entryType[streamFormat.channel[i]]);
        }
    }

    Mesh mesh;
    if (channelTypes[0] == ChannelType::Float3 && channelTypes[1] == ChannelType::Float3 && channelTypes[2] == ChannelType::Float2) {
        BinReaderMemory verticesReader(verticesBuffer.data(), verticesBuffer.size());
        for (int vi = 0; vi < verticesBuffer.size() / bytePerVertex; ++vi) {
            verticesReader.setPosition(vi * bytePerVertex);
            Vector3 position = verticesReader.read<Vector3>();
            Vector3 normal = verticesReader.read<Vector3>();
            Vector2 uv = verticesReader.read<Vector2>();
            mesh.vertices.push_back(position);
            mesh.normals.push_back(normal);
            mesh.uvs.push_back(Vector2{uv.x, 1.0f - uv.y});
        }
    }
    else if (channelTypes[0] == ChannelType::Float3 && channelTypes[1] == ChannelType::Float3 && channelTypes[2] == ChannelType::Color &&
             channelTypes[3] == ChannelType::Float2) {
        BinReaderMemory verticesReader(verticesBuffer.data(), verticesBuffer.size());
        for (int vi = 0; vi < verticesBuffer.size() / bytePerVertex; ++vi) {
            verticesReader.setPosition(vi * bytePerVertex);
            Vector3 position = verticesReader.read<Vector3>();
            Vector3 normal = verticesReader.read<Vector3>();
            int32_t color = verticesReader.read<int32_t>();
            Vector2 uv = verticesReader.read<Vector2>();
            mesh.vertices.push_back(position);
            mesh.normals.push_back(normal);
            mesh.uvs.push_back(Vector2{uv.x, 1.0f - uv.y});
        }
    }
    else if (channelTypes[0] == ChannelType::Float3 && channelTypes[1] == ChannelType::Float2) {
        BinReaderMemory verticesReader(verticesBuffer.data(), verticesBuffer.size());
        for (int vi = 0; vi < verticesBuffer.size() / bytePerVertex; ++vi) {
            verticesReader.setPosition(vi * bytePerVertex);
            Vector3 position = verticesReader.read<Vector3>();
            Vector2 uv = verticesReader.read<Vector2>();
            mesh.vertices.push_back(position);
            mesh.uvs.push_back(Vector2{uv.x, 1.0f - uv.y});
        }
    }
    else if (channelTypes[0] == ChannelType::Float3 && channelTypes[1] == ChannelType::Color && channelTypes[2] == ChannelType::Float2) {
        BinReaderMemory verticesReader(verticesBuffer.data(), verticesBuffer.size());
        for (int vi = 0; vi < verticesBuffer.size() / bytePerVertex; ++vi) {
            verticesReader.setPosition(vi * bytePerVertex);
            Vector3 position = verticesReader.read<Vector3>();
            int32_t color = verticesReader.read<int32_t>();
            Vector2 uv = verticesReader.read<Vector2>();
            mesh.vertices.push_back(position);
            mesh.uvs.push_back(Vector2{uv.x, 1.0f - uv.y});
        }
    }
    else {
        return std::nullopt;
    }

    BinReaderMemory indicesReader(indicesBuffer.data(), indicesBuffer.size());
    for (size_t i = 0; i < indicesBuffer.size() / sizeof(uint16_t); ++i)
        mesh.indices.push_back(indicesReader.read<uint16_t>());

    return mesh;
}

void printFormat(StreamFormat& streamFormat) {
    spdlog::enable_backtrace(16);
    for (int ch = 0; ch < 16; ch++) {
        if (streamFormat.channel[ch] != -1)
            spdlog::debug(entryName[streamFormat.channel[ch]]);
    }
    spdlog::dump_backtrace();

    spdlog::enable_backtrace(16);
    for (int ch = 0; ch < 16; ch++) {
        if (streamFormat.channel[ch] != -1)
            spdlog::debug(entrySize[streamFormat.channel[ch]]);
    }
    spdlog::dump_backtrace();
}

SceneParser::SceneParser(const SirEntry& sirEntry, const std::string& bundleName)
        : sceneRoot(std::nullopt)
        , m_sirEntry(sirEntry) {
    loadScene(sirEntry.sirPath, bundleName);
}

void SceneParser::loadScene(const std::filesystem::path& sirPath, const std::string& bundleName) {
    loadBundle(bundleName);
    addScene(sirPath);
}

void SceneParser::loadBundle(const std::string& bundleName) {
    BundleParser bundleParser(bundlesFolderPath / (bundleName + ".bun"));
    m_bundleName = bundleName;
    m_bundleHeader = bundleParser.parseHeader();
}

void SceneParser::addScene(const std::filesystem::path& sirPath) {
    sceneRoot = loadSir(sirPath);
}

std::optional<SceneNode> SceneParser::loadSir(const std::filesystem::path& sirPath) {
    spdlog::info("Parsing SIR {}...", sirPath.string());
    SharkParser sharkParser(sirPath.string());
    SharkNode* root = sharkParser.getRoot()->goSub("data/root");
    if (root == nullptr) {
        spdlog::error("{} didn't contain 'data/root'", sirPath.string());
        return std::nullopt;
    }

    auto smrPath = sirPath;
    smrPath.replace_extension(".smr");

    auto parsedSir = loadHierarchy(root, smrPath.string(), m_sirEntry.filename);
    if (parsedSir.has_value())
        parsedSir->name = m_sirEntry.filename;

    return parsedSir;
}

std::optional<SceneNode>
SceneParser::loadHierarchy(SharkNode* node, const std::string& smrFile, const std::filesystem::path& hierarchyPath) {
    if (node == nullptr)
        return std::nullopt;

    bool isMmeshLoaded = false;
    SceneNode sceneNode;
    auto position = getEntryArray<float>(node, "transl");
    if (position.has_value())
        sceneNode.position = Vector3{position->at(0), position->at(1), position->at(2)};
    else
        sceneNode.position = Vector3{0.0f, 0.0f, 0.0f};
    auto rotation = getEntryArray<float>(node, "quat");
    if (rotation.has_value())
        sceneNode.rotation = Quaternion{rotation->at(0), rotation->at(1), rotation->at(2), rotation->at(3)};
    else
        sceneNode.rotation = Quaternion{0.0f, 0.0f, 0.0f, 1.0f};
    sceneNode.scale = 1.0f;

    sceneNode.name = *getEntryValue<std::string>(node, "name");
    auto modelName = getEntryValue<std::string>(node, "model");
    auto shader = getEntryValue<std::string>(node, "shader");
    if (modelName.has_value() && shader.has_value()) {
        spdlog::debug("Trying to load {} in {}", *modelName, smrFile);
        auto mesh = loadMesh(smrFile, *modelName, sceneNode.scale);
        if (mesh.has_value()) {
            isMmeshLoaded = true;
            sceneNode.mesh = mesh;

            sceneNode.light = loadLight(*mesh);
        }
    }

    SharkNode* group = node->goSub("child_array");
    if (group != nullptr) {
        // sub_array ?
        for (int i = 0; i < group->count(); ++i) {
            auto child = loadHierarchy(group->at(i), smrFile, hierarchyPath / sceneNode.name);
            if (child.has_value()) {
                isMmeshLoaded = true;
                sceneNode.children.push_back(*child);
            }
        }
    }

    if (isMmeshLoaded || !sceneNode.children.empty())
        return sceneNode;

    return std::nullopt;
}

std::optional<Mesh> SceneParser::loadMesh(const std::string& smrFile, const std::string& modelName, float& outScale) {
    BinReaderMmap binReader(bundlesFolderPath / (m_bundleName + ".bun"));

    BundleHeader& header = m_bundleHeader;

    BundleFileEntry* file = header.getFileEntry(smrFile);
    if (file == nullptr)
        return std::nullopt;

    MeshEntry* meshEntry = file->getMeshEntry(modelName);
    if (meshEntry == nullptr)
        return std::nullopt;

    binReader.setZeroPos(header.posZero);
    binReader.setPosition(meshEntry->posStart + header.posZero);
    MeshInfo info;
    bool success = info.load(binReader);
    if (!success)
        return std::nullopt;

    spdlog::debug("Load part0");
    MeshPartInfo part = info.parts[0];
    if (part.header.formatIdx == 0 || part.header.bitcode == 0 || part.header.numTextures == 0)
        return std::nullopt;

    outScale = info.header.rescale;
    int formatIndex = (part.header.formatIdx / 4 - header.fileEntries.size() - 3) / 18;
    if (header.streamFormats[formatIndex].size == 0)
        return std::nullopt;

    spdlog::debug("Loading vertex data");
    StreamFormat& format = header.streamFormats[formatIndex];
    // printFormat(format);
    VertexDataHeader& data = header.dataHeader[meshEntry->dataIndex];
    int patchVertices = part.header.numVertices / part.header.numAnim;
    if (data.length / data.vertexSize != patchVertices || data.vertexSize / 4 != format.size)
        throw new std::runtime_error("len mismatch : entlen " + std::to_string(data.length) + " entpos " + std::to_string(data.posStart) +
                                     " entsize " + std::to_string(data.vertexSize) + " verts " + std::to_string(patchVertices) +
                                     " frmtsize " + std::to_string(format.size));

    binReader.setPosition(data.posStart);
    std::vector<char> vertices = binReader.readChars(4 * format.size * patchVertices);

    auto mesh = parseMesh(format, vertices, part.indices);
    if (mesh.has_value()) {
        mesh->name = modelName;
        if (modelName.find("skydome") != std::string::npos)
            mesh->smoothness = true;

        mesh->meshParts.resize(part.header.numTexStages);
        int vOffset = 0;
        int iOffset = 0;
        for (int i = 0; i < part.header.numTexStages; ++i) {
            std::vector<std::filesystem::path> texturePath(part.header.numTextures);
            for (int l = 0; l < part.header.numTextures; ++l) {
                texturePath[l] = (part.tex[l].texIdx[i] == -1) ? "" : header.textures[info.texIdx[part.tex[l].texIdx[i]]];
            }
            std::filesystem::path exportPath = std::filesystem::path("meshes/textures") / m_bundleName;
            parseTextures(mesh->meshParts[i], texturePath, exportPath);

            mesh->meshParts[i].vertexInterval = std::pair(vOffset, vOffset + part.stageVertices[i]);
            mesh->meshParts[i].indexInterval = std::pair(iOffset, iOffset + part.stageIndices[i]);
            vOffset += part.stageVertices[i];
            iOffset += part.stageIndices[i];
        }
    }

    return mesh;
}

std::optional<PointLight> SceneParser::loadLight(const Mesh& mesh) {
    if (mesh.name.find("lampglow") == std::string::npos)
        return std::nullopt;

    assert(!mesh.meshParts.empty() && !mesh.meshParts[0].textures.empty());

    const auto& texturePath = mesh.meshParts[0].textures[0];

    int x, y, component;
    unsigned char* data = stbi_load(texturePath.string().c_str(), &x, &y, &component, 0);
    int numberOfPixels = x * y;
    Vector3 averageColor{0.0, 0.0, 0.0};
    for (int i = 0; i < numberOfPixels; ++i) {
        averageColor.x += data[i * 4 + 0];
        averageColor.y += data[i * 4 + 1];
        averageColor.z += data[i * 4 + 2];
    }
    stbi_image_free(data);

    averageColor.x /= numberOfPixels;
    averageColor.y /= numberOfPixels;
    averageColor.z /= numberOfPixels;

    Vector3 bmin = mesh.vertices[0];
    Vector3 bmax = mesh.vertices[0];
    for (const Vector3& v : mesh.vertices) {
        bmin.x = std::min(bmin.x, v.x);
        bmin.y = std::min(bmin.y, v.y);
        bmin.z = std::min(bmin.z, v.z);
        bmax.x = std::max(bmax.x, v.x);
        bmax.y = std::max(bmax.y, v.y);
        bmax.z = std::max(bmax.z, v.z);
    }

    Vector3 center{0.5f * (bmin.x + bmax.x), 0.5f * (bmin.y + bmax.y), 0.5f * (bmin.z + bmax.z)};
    float intencity = std::max(bmax.x - bmin.x, std::max(bmax.y - bmin.y, bmax.z - bmin.z)) * 10;

    return PointLight{averageColor, center, intencity};
}

} // namespace parser