#include "Scene.h"
#include "BinReader.h"
#include "BUNParser.h"
#include "GeometryData.h"
#include "PAKParser.h"
#include "SharkNode.h"
#include "SceneNode.h"
#include "SharkParser.h"

#include <fstream>
#include <iostream>
#include <array>

namespace parser
{

const enum class ChannelType {
    Unused,
    Float2,
    Float3,
    Float4,
    Color
};
const size_t entrySize[] = { 0, 8, 12, 16, 4 };
const std::string entryName[] = { "Unused", "Float2", "Float3", "Float4", "Color" };
const ChannelType entryType[] = { ChannelType::Unused, ChannelType::Float2, ChannelType::Float3, ChannelType::Float4, ChannelType::Color };

//Textures parseTextures(std::vector<std::string> texturePath) {
//    Textures textures;
//    for (int i = 0; i < texturePath.size(); ++i) {
//        PAKParser::instance().tryExtract(texturePath[i]);
//    }
//    return textures;
//}

std::optional<Mesh> parseMesh(const StreamFormat& streamFormat, const std::vector<char>& verticesBuffer, const std::vector<char>& indicesBuffer)
{
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

    if (!(channelTypes[0] == ChannelType::Float3 && channelTypes[1] == ChannelType::Float3 && channelTypes[2] == ChannelType::Float2))
        return std::nullopt;

    Mesh mesh;
    BinReader verticesReader(verticesBuffer);
    for (int vi = 0; vi < verticesBuffer.size() / bytePerVertex; ++vi) {
        verticesReader.setPosition(vi * bytePerVertex);
        Vector3 position = verticesReader.read<Vector3>();
        Vector3 normal = verticesReader.read<Vector3>();
        Vector2 uv = verticesReader.read<Vector2>();
        mesh.vertices.push_back(position);
        mesh.normals.push_back(normal);
        mesh.uvs.push_back(uv);
    }

    BinReader indicesReader(indicesBuffer);
    for (size_t i = 0; i < indicesBuffer.size() / sizeof(uint16_t); ++i)
        mesh.indices.push_back(indicesReader.readUint16());

    return mesh;
}

void printFormat(StreamFormat& streamFormat)
{
    for (int ch = 0; ch < 16; ch++)
    {
        if (streamFormat.channel[ch] != -1) {
            std::cout << entryName[streamFormat.channel[ch]] << " ";
        }
    }
    std::cout << std::endl;

    for (int ch = 0; ch < 16; ch++)
    {
        if (streamFormat.channel[ch] != -1) {
            std::cout << entrySize[streamFormat.channel[ch]] << " ";
        }
    }
    std::cout << std::endl;
}

Scene::Scene(const std::filesystem::path& sirPath, const std::string& bundleName)
    : sceneRoot(std::nullopt)
{
    loadScene(sirPath, bundleName);
}

void Scene::loadScene(const std::filesystem::path& sirPath, const std::string& bundleName) {
    loadBundle(bundleName);
    addScene(sirPath);
}

void Scene::loadBundle(const std::string& bundleName) {
    BUNParser bunParser("bundles/" + bundleName + ".bun");
    m_bundleName = bundleName;
    m_bundleHeader = bunParser.parseHeader();
}

void Scene::addScene(const std::filesystem::path& sirPath) {
    sceneRoot = loadSir(sirPath);
}

std::optional<SceneNode> Scene::loadSir(const std::filesystem::path& sirPath) {
    std::cout << "Parsing sir " << sirPath.string() << std::endl;
    SharkParser cdrParser(sirPath.string());
    SharkNode* root = cdrParser.getRoot()->goSub("data/root");
    if (root == nullptr)
    {
        std::cerr << sirPath.string() << " didn't contain 'data/root'" << std::endl;
        return std::nullopt;
    }

    auto smrPath = sirPath;
    smrPath.replace_extension(".smr");

    return loadHierarchy(root, smrPath.string());
}

std::optional<SceneNode> Scene::loadHierarchy(SharkNode* node, const std::string& smrFile) {
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
        sceneNode.rotation = Quaternion{0.0f, 0.0f, 0.0f, 0.0f};
    sceneNode.scale = 1.0f;

    auto name = getEntryValue<std::string>(node, "model");
    auto shader = getEntryValue<std::string>(node, "shader");
    if (name.has_value() && shader.has_value())
    {
        std::cout << "Trying to load " << smrFile << " * " << *name << std::endl;
        auto mesh = loadMesh(smrFile, *name, sceneNode.scale);
        if (mesh.has_value()) {
            isMmeshLoaded = true;
            sceneNode.mesh = mesh;
        }
    }

    SharkNode* group = node->goSub("child_array");
    if (group != nullptr) {
        // sub_array ?
        for (int i = 0; i < group->count(); ++i)
        {
            auto child = loadHierarchy(group->at(i), smrFile);
            if (child.has_value()) {
                isMmeshLoaded = true;
                sceneNode.children.push_back(*child);
            }
        }
    }

    if (isMmeshLoaded)
        return sceneNode;

    return std::nullopt;
}

std::optional<Mesh> Scene::loadMesh(const std::string& smrFile, const std::string& modelName, float& outScale) {
    BinReader binReader("bundles/" + m_bundleName + ".bun");

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

    std::cout << "Load part0" << std::endl;
    MeshPart part = info.parts[0];
    if (part.header.formatIdx == 0 || part.header.bitcode == 0 || part.header.numTextures == 0)
        return std::nullopt;

    outScale = info.header.rescale;
    int formatIndex = (part.header.formatIdx / 4 - header.fileEntries.size() - 3) / 18;
    if (header.streamFormats[formatIndex].size == 0)
        return std::nullopt;

    std::cout << "Loading vertex data" << std::endl;
    StreamFormat& format = header.streamFormats[formatIndex];
    //printFormat(format);
    VertexDataHeader& data = header.dataHeader[meshEntry->dataIndex];
    int patchVertices = part.header.numVertices / part.header.numAnim;
    if (data.length / data.vertexSize != patchVertices || data.vertexSize / 4 != format.size)
        throw new std::runtime_error("len mismatch : entlen " + std::to_string(data.length) + 
                                     " entpos " + std::to_string(data.posStart) + 
                                     " entsize " + std::to_string(data.vertexSize) + 
                                     " verts " + std::to_string(patchVertices) + 
                                     " frmtsize " + std::to_string(format.size));

    binReader.setPosition(data.posStart);
    std::vector<char> vertices = binReader.readChars(4 * format.size * patchVertices);

    auto mesh = parseMesh(format, vertices, part.indices);
    if (mesh.has_value()) {
        std::cout << "Mesh parsed successfully" << std::endl;
        std::cout << "Adding textures" << std::endl;
        for (int i = 0; i < part.header.numTexStages; i++)
        {
            std::vector<std::string> texturePath(part.header.numTextures);
            for (int l = 0; l < part.header.numTextures; l++) {
                texturePath[l] = (part.tex[l].texIdx[i] == -1) ? "" : header.textures[info.texIdx[part.tex[l].texIdx[i]]];
            }
            //mesh->textureStage.push_back(parseTextures(texturePath));
        }
    }

    return mesh;
}

}