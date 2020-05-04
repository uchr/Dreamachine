#pragma once

#include "BundleHeader.h"

#include <filesystem>

struct SharkNode;

struct Mesh {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<int> indices;
};

class Scene
{
public:
    Scene(const std::filesystem::path& sirPath, const std::string& bundleName);

    std::vector<Mesh> meshes;

private:
    void loadScene(const std::filesystem::path& sirPath, const std::string& bundleName);
    void loadBundle(const std::string& bundleName);
    void addScene(const std::filesystem::path& sirPath);

    std::vector<Mesh> loadSir(const std::filesystem::path& sirPath);
    std::vector<Mesh> loadHierarchy(SharkNode* node, const std::string& smrFile);
    std::optional<Mesh> loadMesh(const std::string& smrFile, const std::string& modelName);

    BundleHeader m_bundleHeader;
    std::string m_bundleName;
};