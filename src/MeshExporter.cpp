#include "MeshExporter.h"
#include "TextureParser.h"
#include "SceneNode.h"
#include "Utils.h"

#include <fbxsdk.h>
#include <spdlog/spdlog.h>

void createScene(FbxManager* manager, FbxScene* scene, FbxNode* parent, const parser::SceneNode& node)
{
    auto createTexture = [&](const std::filesystem::path& path) {
        FbxFileTexture* texture = FbxFileTexture::Create(manager, path.string().c_str());

        texture->SetFileName(path.string().c_str());
        texture->SetTextureUse(FbxTexture::eStandard);
        texture->SetMappingType(FbxTexture::eUV);
        texture->SetMaterialUse(FbxFileTexture::eModelMaterial);
        texture->SetSwapUV(false);
        texture->SetTranslation(0.0, 0.0);
        texture->SetScale(1.0, 1.0);
        texture->SetRotation(0.0, 0.0);
        texture->UVSet.Set(FbxString("DiffuseUV"));

        return texture;
    };

    auto createMaterial = [&](const parser::MeshPart& meshPart) {
        FbxString materialName = Utils::getFileNameWithoutExtension(meshPart.textures[0]).c_str();

        // Create material
        FbxString shadingName = "Phong";
        FbxSurfacePhong* material = FbxSurfacePhong::Create(manager, materialName.Buffer());
        material->Ambient.Set(FbxDouble3(0.8, 0.8, 0.8));

        const auto& diffuseTexturePath = meshPart.textures[0];
        FbxFileTexture* diffuseTexture = createTexture(diffuseTexturePath);
        material->Diffuse.ConnectSrcObject(diffuseTexture);

        if (meshPart.alphaTexture.has_value())
            material->TransparentColor.ConnectSrcObject(createTexture(meshPart.alphaTexture.value()));

        auto normalMapIt = std::find_if(meshPart.textures.begin(), meshPart.textures.end(),
            [](const std::filesystem::path& path) { return path.string().find("nml") != std::string::npos; } );
        if (normalMapIt != meshPart.textures.end())
            material->NormalMap.ConnectSrcObject(createTexture(*normalMapIt));

        auto heightMapIt = std::find_if(meshPart.textures.begin(), meshPart.textures.end(),
            [](const std::filesystem::path& path) { return path.string().find("hmap") != std::string::npos; } );
        if (heightMapIt != meshPart.textures.end()) {
            material->Bump.ConnectSrcObject(createTexture(*heightMapIt));
            material->BumpFactor.Set(1.0);
        }

        return material;
    };

    FbxNode* fbxMeshNode = FbxNode::Create(manager, node.name.c_str());
    parent->AddChild(fbxMeshNode);

    const double rad2Deg = 57.2958;
    parser::Transofrmation transformation = node.computeTransformation();
    fbxMeshNode->LclTranslation.Set(FbxVector4(transformation.translation.x, transformation.translation.y, transformation.translation.z));
    fbxMeshNode->LclRotation.Set(FbxVector4(transformation.rotation.x * rad2Deg, transformation.rotation.y * rad2Deg, transformation.rotation.z * rad2Deg));
    fbxMeshNode->LclScaling.Set(FbxVector4(transformation.scale, transformation.scale, transformation.scale));

    if (node.mesh.has_value() && !node.mesh->meshParts.empty() && !node.mesh->meshParts[0].textures.empty()) {
        const auto& mesh = *node.mesh;
        FbxMesh* fbxMesh = FbxMesh::Create(manager, node.name.c_str());
        fbxMesh->InitControlPoints(mesh.vertices.size());
        FbxVector4* controlPoints = fbxMesh->GetControlPoints();

        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            const parser::Vector3& v = mesh.vertices[i];
            controlPoints[i] = FbxVector4(v.x, v.y, v.z);
        }

        FbxLayerElementNormal* layerNormal = nullptr;
        if (!mesh.normals.empty()) {
            layerNormal = FbxLayerElementNormal::Create(fbxMesh, "");
            layerNormal->SetMappingMode(FbxLayerElement::eByControlPoint);
            layerNormal->SetReferenceMode(FbxLayerElement::eDirect);

            for (size_t i = 0; i < mesh.normals.size(); i++) {
                const parser::Vector3& n = mesh.normals[i];
                layerNormal->GetDirectArray().Add(FbxVector4(n.x, n.y, n.z));
            }
        }

        FbxGeometryElementSmoothing* smoothingElement = nullptr;
        if (mesh.smoothness) {
            smoothingElement = FbxGeometryElementSmoothing::Create(fbxMesh, "");
            smoothingElement->SetMappingMode(FbxLayerElement::eByPolygon);
            smoothingElement->SetReferenceMode(FbxLayerElement::eDirect);

            for (size_t i = 0; i < mesh.indices.size() / 3; i++) {
                smoothingElement->GetDirectArray().Add(true);
            }
        }

        FbxLayerElementUV* layerTexcoord = nullptr;
        if (!mesh.uvs.empty()) {
            layerTexcoord = FbxLayerElementUV::Create(fbxMesh, "DiffuseUV");
            layerTexcoord->SetMappingMode(FbxLayerElement::eByControlPoint);
            layerTexcoord->SetReferenceMode(FbxLayerElement::eDirect);

            for (size_t i = 0; i < mesh.uvs.size(); i++) {
                const parser::Vector2& uv = mesh.uvs[i];
                layerTexcoord->GetDirectArray().Add(FbxVector2(uv.x, uv.y));
            }
        }

        for (size_t partIndex = 0; partIndex < mesh.meshParts.size(); ++partIndex) {
            const auto& meshPart = mesh.meshParts[partIndex];
            FbxLayer* layer = fbxMesh->GetLayer(partIndex);
            if (!layer) {
                fbxMesh->CreateLayer();
                layer = fbxMesh->GetLayer(partIndex);
            }

            if (layerNormal != nullptr)
                layer->SetNormals(layerNormal);

            if (smoothingElement != nullptr)
                layer->SetSmoothing(smoothingElement);

            if (layerTexcoord != nullptr) {
                layer->SetUVs(layerTexcoord, FbxLayerElement::eTextureDiffuse);

                fbxMeshNode->AddMaterial(createMaterial(meshPart));
                FbxLayerElementMaterial* layerMaterial = FbxLayerElementMaterial::Create(fbxMesh, "");
                layerMaterial->SetMappingMode(FbxLayerElement::eByPolygon);
                layerMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);
                layer->SetMaterials(layerMaterial);
            }

            // Create polygons
            for (size_t ti = meshPart.indexInterval.first; ti < meshPart.indexInterval.second;)
            {
                fbxMesh->BeginPolygon(fbxMeshNode->GetMaterialCount() - 1);

                fbxMesh->AddPolygon(mesh.indices[ti++]);
                fbxMesh->AddPolygon(mesh.indices[ti++]);
                fbxMesh->AddPolygon(mesh.indices[ti++]);

                fbxMesh->EndPolygon();
            }
        }

        fbxMeshNode->SetNodeAttribute(fbxMesh);
        fbxMeshNode->SetShadingMode(FbxNode::eTextureShading);
    }

    if (!node.children.empty()) {
        for (size_t i = 0; i < node.children.size(); ++i)
            createScene(manager, scene, fbxMeshNode, node.children[i]);
    }
}

bool saveScene(FbxManager* manager, FbxDocument* scene, const std::string& filename, int fileFormat)
{
    FbxExporter* exporter = FbxExporter::Create(manager, "");

    if (!exporter->Initialize(filename.c_str(), fileFormat, manager->GetIOSettings()))
    {
        spdlog::error("Call to FbxExporter::Initialize() failed.");
        spdlog::error("Error returned: {} \n\n", exporter->GetStatus().GetErrorString());
        return false;
    }

    bool status = exporter->Export(scene);
    exporter->Destroy();
    return status;
}

bool exportScene(const parser::SceneNode& root, const std::string& bundleName, const std::string& meshName) {
    std::filesystem::path path = std::filesystem::path("meshes") / bundleName / meshName / (meshName + ".fbx");
    std::filesystem::create_directories(path.parent_path());

    FbxManager* manager = FbxManager::Create();
    if (!manager)
    {
        spdlog::error("Error: Unable to create FBX Manager!");
        return false;
    }
    else {
        spdlog::debug("Autodesk FBX SDK version {}", manager->GetVersion());
    }

    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);

    ios->SetBoolProp(EXP_FBX_MATERIAL, true);
    ios->SetBoolProp(EXP_FBX_TEXTURE, true);
    ios->SetBoolProp(EXP_FBX_SHAPE, true);
    ios->SetBoolProp(EXP_FBX_GOBO, true);
    ios->SetBoolProp(EXP_FBX_ANIMATION, false);
    ios->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

    FbxString applicationPath = FbxGetApplicationDirectory();
    manager->LoadPluginsDirectory(applicationPath.Buffer());

    FbxScene* scene = FbxScene::Create(manager, "Dreamachine Scene");
    if (!scene)
    {
        spdlog::error("Error: Unable to create FBX scene!");
        return false;
    }

    createScene(manager, scene, scene->GetRootNode(), root);
    saveScene(manager, scene, path.string().c_str(), -1);

    manager->Destroy();
    return true;
}
