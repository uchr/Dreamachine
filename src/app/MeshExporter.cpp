#include "MeshExporter.h"

#include <parser/TextureParser.h>
#include <parser/SceneNode.h>
#include <parser/Utils.h>

#include <fbxsdk.h>
#include <spdlog/spdlog.h>

void prepareScene(FbxManager* fbxManager, FbxScene* fbxScene, FbxNode* fbxSceneNode, const parser::SceneNode& parsedSceneNode, bool isRootNode = false) {
    auto createTexture = [&](const std::filesystem::path& path) {
        const std::string textureName = Utils::getFilenameWithoutExtension(path);
        FbxFileTexture* texture = FbxFileTexture::Create(fbxManager, textureName.c_str());

        texture->SetFileName(path.string().c_str());
        texture->SetName(textureName.c_str());
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
        const std::string materialName = Utils::getFilenameWithoutExtension(meshPart.textures[0]);
        FbxSurfacePhong* material = FbxSurfacePhong::Create(fbxManager, materialName.c_str());
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

    FbxNode* fbxMeshNode = FbxNode::Create(fbxManager, parsedSceneNode.name.c_str());
    fbxSceneNode->AddChild(fbxMeshNode);

    const double rad2Deg = 57.2958;
    parser::Transofrmation transformation = parsedSceneNode.computeTransformation();
    fbxMeshNode->LclTranslation.Set(FbxVector4(transformation.translation.x, transformation.translation.y, transformation.translation.z));
    fbxMeshNode->LclRotation.Set(FbxVector4(transformation.rotation.x * rad2Deg, transformation.rotation.y * rad2Deg, transformation.rotation.z * rad2Deg));
    fbxMeshNode->LclScaling.Set(FbxVector4(transformation.scale, transformation.scale, transformation.scale));

    if (isRootNode) {
        fbxMeshNode->LclRotation.Set(FbxVector4(-90, 0, 0));
        fbxMeshNode->LclScaling.Set(FbxVector4(50, 50, 50));
    }

    if (parsedSceneNode.mesh.has_value() && !parsedSceneNode.mesh->meshParts.empty() && !parsedSceneNode.mesh->meshParts[0].textures.empty()) {
        const auto& mesh = *parsedSceneNode.mesh;
        FbxMesh* fbxMesh = FbxMesh::Create(fbxManager, parsedSceneNode.name.c_str());
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

    if (parsedSceneNode.light.has_value()) {
        const auto& light = *parsedSceneNode.light;
        FbxNode* fbxLightNode = FbxNode::Create(fbxScene, "PointLightNode");
        fbxMeshNode->AddChild(fbxLightNode);

        FbxLight* fbxLight = FbxLight::Create(fbxScene, "PointLight");
        fbxLightNode->SetNodeAttribute(fbxLight);
        fbxLight->LightType.Set(FbxLight::ePoint);
        fbxLight->Color.Set(FbxDouble3(light.color.x / 255.0, light.color.y / 255.0, light.color.z / 255.0));
        fbxLight->Intensity.Set(light.intencity);
    }

    if (!parsedSceneNode.children.empty()) {
        for (size_t i = 0; i < parsedSceneNode.children.size(); ++i)
            prepareScene(fbxManager, fbxScene, fbxMeshNode, parsedSceneNode.children[i]);
    }
}

bool saveScene(FbxManager* fbxManager, FbxScene* fbxScene, const std::string& outputPath, int fileFormat)
{
    FbxExporter* exporter = FbxExporter::Create(fbxManager, "");

    if (!exporter->Initialize(outputPath.c_str(), fileFormat, fbxManager->GetIOSettings()))
    {
        spdlog::error("Call to FbxExporter::Initialize() failed.");
        spdlog::error("Error returned: {} \n\n", exporter->GetStatus().GetErrorString());
        return false;
    }

    bool status = exporter->Export(fbxScene);
    exporter->Destroy();
    return status;
}

bool exportScene(const std::vector<parser::SceneNode>& parsedSceneNodes, const std::filesystem::path& outputPath, ExportMode exportMode) {
    if (parsedSceneNodes.empty())
        return false;

    FbxManager* fbxManager = FbxManager::Create();
    if (!fbxManager)
    {
        spdlog::error("Error: Unable to create FBX Manager!");
        return false;
    }
    else {
        spdlog::debug("Autodesk FBX SDK version {}", fbxManager->GetVersion());
    }

    FbxIOSettings* ios = FbxIOSettings::Create(fbxManager, IOSROOT);
    fbxManager->SetIOSettings(ios);

    ios->SetBoolProp(EXP_FBX_MATERIAL, true);
    ios->SetBoolProp(EXP_FBX_TEXTURE, true);
    ios->SetBoolProp(EXP_FBX_SHAPE, true);
    ios->SetBoolProp(EXP_FBX_GOBO, true);
    ios->SetBoolProp(EXP_FBX_ANIMATION, false);
    ios->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

    FbxString applicationPath = FbxGetApplicationDirectory();
    fbxManager->LoadPluginsDirectory(applicationPath.Buffer());

    FbxScene* fbxScene = FbxScene::Create(fbxManager, "Dreamachine Scene");
    if (!fbxScene)
    {
        spdlog::error("Error: Unable to create FBX scene!");
        return false;
    }

    if (exportMode == ExportMode::Single) {
        for (const auto& parsedSceneRoot : parsedSceneNodes)
            prepareScene(fbxManager, fbxScene, fbxScene->GetRootNode(), parsedSceneRoot, true);

        std::filesystem::create_directories(outputPath.parent_path());
        saveScene(fbxManager, fbxScene, outputPath.string().c_str(), -1);
    }
    else if (exportMode == ExportMode::Multiple) {
        std::filesystem::create_directories(outputPath);
        for (const auto& parsedSceneRoot : parsedSceneNodes) {
            prepareScene(fbxManager, fbxScene, fbxScene->GetRootNode(), parsedSceneRoot, true);

            auto meshPath = outputPath / (parsedSceneRoot.name + ".fbx");
            saveScene(fbxManager, fbxScene, meshPath.string().c_str(), -1);
        }
    }

    fbxManager->Destroy();
    return true;
}
