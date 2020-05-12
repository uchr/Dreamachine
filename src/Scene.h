#pragma once

#include "BundleHeader.h"
#include "SceneNode.h"
#include "SceneIndex.h"

#include <filesystem>

namespace parser
{

struct SharkNode;

class Scene
{
public:
    Scene(const SirEntry& sirEntry, const std::string& bundleName);

    std::optional<SceneNode> sceneRoot;

private:
    void loadScene(const std::filesystem::path& sirPath, const std::string& bundleName);
    void loadBundle(const std::string& bundleName);
    void addScene(const std::filesystem::path& sirPath);

    std::optional<SceneNode> loadSir(const std::filesystem::path& sirPath);
    std::optional<SceneNode> loadHierarchy(SharkNode* node, const std::string& smrFile, const std::filesystem::path& hierarchyPath);
    std::optional<Mesh> loadMesh(const std::string& smrFile, const std::filesystem::path& hierarchyPath, const std::string& modelName, float& outScale);

    BundleHeader m_bundleHeader;
    std::string m_bundleName;
    const SirEntry& m_sirEntry;
};

}