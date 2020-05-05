#pragma once

#include "BundleHeader.h"
#include "SceneNode.h"

#include <filesystem>

struct SharkNode;

class Scene
{
public:
    Scene(const std::filesystem::path& sirPath, const std::string& bundleName);

    std::optional<SceneNode> sceneRoot;

private:
    void loadScene(const std::filesystem::path& sirPath, const std::string& bundleName);
    void loadBundle(const std::string& bundleName);
    void addScene(const std::filesystem::path& sirPath);

    std::optional<SceneNode> loadSir(const std::filesystem::path& sirPath);
    std::optional<SceneNode> loadHierarchy(SharkNode* node, const std::string& smrFile);
    std::optional<Mesh> loadMesh(const std::string& smrFile, const std::string& modelName, float& outScale);

    BundleHeader m_bundleHeader;
    std::string m_bundleName;
};