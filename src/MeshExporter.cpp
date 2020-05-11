#include "MeshExporter.h"
#include "SceneNode.h"
#include "Utils.h"

#include <fbxsdk.h>

#include <iostream>

void createScene(FbxManager* manager, FbxScene* scene, FbxNode* parent, const parser::SceneNode& node)
{
    auto createMaterial = [&](const parser::Mesh& mesh) {
        FbxString materialName = Utils::getFileNameWithoutExtension(mesh.texturePath[0]).c_str();

        // Create material
        FbxString shadingName = "Phong";
        FbxSurfacePhong* material = FbxSurfacePhong::Create(manager, materialName.Buffer());
        material->Ambient.Set(FbxDouble3(0.8, 0.8, 0.8));

        // Create texture
        std::filesystem::path texturePath = "./" / mesh.texturePath[0].filename();
        FbxFileTexture* texture = FbxFileTexture::Create(manager, texturePath.string().c_str());
        texture->SetFileName(texturePath.string().c_str());
        texture->SetTextureUse(FbxTexture::eStandard);
        texture->SetMappingType(FbxTexture::eUV);
        texture->SetMaterialUse(FbxFileTexture::eModelMaterial);
        texture->SetSwapUV(false);
        texture->SetTranslation(0.0, 0.0);
        texture->SetScale(1.0, 1.0);
        texture->SetRotation(0.0, 0.0);
        texture->UVSet.Set(FbxString("DiffuseUV"));
        material->Diffuse.ConnectSrcObject(texture);

        return material;
    };

    FbxNode* fbxMeshNode = FbxNode::Create(manager, node.name.c_str());
    parent->AddChild(fbxMeshNode);

    parser::Transofrmation transformation = node.computeTransformation();
    fbxMeshNode->LclTranslation.Set(FbxVector4(transformation.translation.x, transformation.translation.y, transformation.translation.z));
    fbxMeshNode->LclRotation.Set(FbxVector4(transformation.rotation.x * 57.2958, transformation.rotation.y * 57.2958, transformation.rotation.z * 57.2958));
    fbxMeshNode->LclScaling.Set(FbxVector4(transformation.scale, transformation.scale, transformation.scale));

    if (node.mesh.has_value() && !node.mesh->texturePath.empty()) {
        const auto& mesh = *node.mesh;
        FbxMesh* fbxMesh = FbxMesh::Create(manager, node.name.c_str());
        fbxMesh->InitControlPoints(mesh.vertices.size());
        FbxVector4* controlPoints = fbxMesh->GetControlPoints();

        // Create normal layer
        FbxLayer* layer = fbxMesh->GetLayer(0);
        if (!layer) {
            fbxMesh->CreateLayer();
            layer = fbxMesh->GetLayer(0);
        }

        // Setup layers
        FbxLayerElementNormal* layerNormal = FbxLayerElementNormal::Create(fbxMesh, "");
        layerNormal->SetMappingMode(FbxLayerElement::eByControlPoint);
        layerNormal->SetReferenceMode(FbxLayerElement::eDirect);

        FbxLayerElementUV* layerTexcoord = FbxLayerElementUV::Create(fbxMesh, "DiffuseUV");
        layerTexcoord->SetMappingMode(FbxLayerElement::eByControlPoint);
        layerTexcoord->SetReferenceMode(FbxLayerElement::eDirect);
        layer->SetUVs(layerTexcoord, FbxLayerElement::eTextureDiffuse);

        // Fill data
        for (size_t i = 0; i < mesh.vertices.size(); i++)
        {
            const parser::Vector3& v = mesh.vertices[i];
            const parser::Vector3& n = mesh.normals[i];
            const parser::Vector2& uv = mesh.uvs[i];

            controlPoints[i] = FbxVector4(v.x, v.y, v.z);
            layerNormal->GetDirectArray().Add(FbxVector4(n.x, n.y, n.z));
            layerTexcoord->GetDirectArray().Add(FbxVector2(uv.x, uv.y));
        }
        layer->SetNormals(layerNormal);

        // Create material
        FbxLayerElementMaterial* layerMaterial = FbxLayerElementMaterial::Create(fbxMesh, "");
        layerMaterial->SetMappingMode(FbxLayerElement::eByPolygon);
        layerMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);
        layer->SetMaterials(layerMaterial);

        // Create polygons
        for (size_t ti = 0; ti < mesh.indices.size();)
        {
            fbxMesh->BeginPolygon(0);

            fbxMesh->AddPolygon(mesh.indices[ti++]);
            fbxMesh->AddPolygon(mesh.indices[ti++]);
            fbxMesh->AddPolygon(mesh.indices[ti++]);

            fbxMesh->EndPolygon();
        }

        fbxMeshNode->AddMaterial(createMaterial(mesh));
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
        std::cerr << "Call to FbxExporter::Initialize() failed." << std::endl;
        std::cerr << "Error returned:" << exporter->GetStatus().GetErrorString() << std::endl << std::endl;
        return false;
    }

    bool status = exporter->Export(scene);
    exporter->Destroy();
    return status;
}

bool exportScene(const parser::SceneNode& root, const std::filesystem::path& path) {
    FbxManager* manager = FbxManager::Create();
    if (!manager)
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        return false;
    }
    else {
        FBXSDK_printf("Autodesk FBX SDK version %s\n", manager->GetVersion());
    }

    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);

    ios->SetBoolProp(EXP_FBX_MATERIAL, true);
    ios->SetBoolProp(EXP_FBX_TEXTURE, true);
    ios->SetBoolProp(EXP_FBX_EMBEDDED, true);
    ios->SetBoolProp(EXP_FBX_SHAPE, true);
    ios->SetBoolProp(EXP_FBX_GOBO, true);
    ios->SetBoolProp(EXP_FBX_ANIMATION, false);
    ios->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

    FbxString applicationPath = FbxGetApplicationDirectory();
    manager->LoadPluginsDirectory(applicationPath.Buffer());

    FbxScene* scene = FbxScene::Create(manager, "Dreamachine Scene");
    if (!scene)
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        return false;
    }

    createScene(manager, scene, scene->GetRootNode(), root);
    saveScene(manager, scene, path.string().c_str(), -1);

    manager->Destroy();
    return true;
}
