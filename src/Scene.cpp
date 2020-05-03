#include "Scene.h"
#include "CDRParser.h"
#include "BUNParser.h"
#include "SceneNode.h"
#include "BinReader.h"

#include <fstream>
#include <iostream>
#include <array>

std::optional<Mesh> parseMesh(const StreamFormat& streamFormat, const std::vector<char>& verticesBuffer, const std::vector<char>& indicesBuffer)
{
    Mesh mesh;

    int numberOfChannel = 0;
    for (int i = 0; i < 16; ++i)
        if (streamFormat.channel[i] != -1)
            ++numberOfChannel;

    if (numberOfChannel != 5) // TODO: parse all formats
        return std::nullopt;

    BinReader verticesReader(verticesBuffer);
    while(!verticesReader.isEnd()) { // TODO: use size of vertices
        Vector3 position = verticesReader.read<Vector3>();
        Vector3 normal = verticesReader.read<Vector3>();
        Vector2 uv = verticesReader.read<Vector2>();
        Vector3 unkown0 = verticesReader.read<Vector3>();
        Vector3 unkown1 = verticesReader.read<Vector3>();
        mesh.vertices.push_back(position);
        mesh.normals.push_back(normal);
        mesh.uvs.push_back(uv);
    }

    BinReader indicesReader(indicesBuffer);
    for (size_t i = 0; i < indicesBuffer.size() / 2; ++i)
        mesh.indices.push_back(indicesReader.readUint16());

    return mesh;
}

void printFormat(StreamFormat& streamFormat)
{
    size_t entrySize[] = { 0, 8, 12, 16, 4 };
    std::string entryType[] = { "Unused", "Float2", "Float3", "Float4", "Color" };
    for (int ch = 0; ch < 16; ch++)
    {
        if (streamFormat.channel[ch] != -1) {
            std::cout << entryType[streamFormat.channel[ch]] << " ";
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

Scene::Scene(const std::filesystem::path& sirPath, const std::string& bundleName) {
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
    meshes = loadSir(sirPath);
}

std::vector<Mesh> Scene::loadSir(const std::filesystem::path& sirPath) {
    std::cout << "Parsing sir " << sirPath.string() << std::endl;
    CDRParser cdrParser(sirPath.string());
    SceneNode* root = cdrParser.getRoot()->goSub("data/root");
    if (root == nullptr)
    {
        std::cerr << sirPath.string() << " didn't contain 'data/root'" << std::endl;
        return {};
    }
    auto smrPath = sirPath;
    smrPath.replace_extension(".smr");
    return loadHierarchy(root, smrPath.string());
}

std::vector<Mesh> Scene::loadHierarchy(SceneNode* node, const std::string& smrFile) {
    if (node == nullptr)
        return {};

    std::vector<Mesh> loadedMeshes;
    auto name = getEntryValue<std::string>(node, "model");
    auto shader = getEntryValue<std::string>(node, "shader");
    if (name.has_value() && shader.has_value())
    {
        std::cout << "Trying to load " << smrFile << " * " << *name << std::endl;
        auto mesh = loadMesh(smrFile, *name);
        if (mesh.has_value())
            loadedMeshes.push_back(std::move(*mesh));
    }

    SceneNode* group = node->goSub("child_array");
    if (group != nullptr) {
        // sub_array ?
        for (int i = 0; i < group->count(); ++i)
        {
            auto child = loadHierarchy(group->at(i), smrFile);
            if (!child.empty())
            {
                loadedMeshes.reserve(loadedMeshes.size() + child.size());
                std::move(std::begin(child), std::end(child), std::back_inserter(loadedMeshes));
            }
        }
    }

    return loadedMeshes;
}

std::optional<Mesh> Scene::loadMesh(const std::string& smrFile, const std::string& modelName) {
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

    float rescale = 1.0f / info.header.rescale;
    int formatIndex = (part.header.formatIdx / 4 - header.fileEntries.size() - 3) / 18;
    if (header.streamFormats[formatIndex].size == 0)
        return std::nullopt;

    std::cout << "Loading vertex data" << std::endl;
    StreamFormat& format = header.streamFormats[formatIndex];
    printFormat(format);
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
    return parseMesh(format, vertices, part.indices);
}
