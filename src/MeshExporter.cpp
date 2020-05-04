#include "MeshExporter.h"

#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>

#include <fstream>

aiScene generateScene(const Mesh& mesh);

void exportFBX(const Mesh& mesh, const std::filesystem::path& path) {
    aiScene scene;

    scene.mRootNode = new aiNode();

    scene.mMaterials = new aiMaterial*[1];
    scene.mNumMaterials = 1;

    scene.mMaterials[0] = new aiMaterial();

    if (true) {
        scene.mMeshes = new aiMesh*[1];
        scene.mNumMeshes = 1;

        scene.mMeshes[0] = new aiMesh();
        scene.mMeshes[0]->mMaterialIndex = 0;

        scene.mRootNode->mMeshes = new unsigned int[1];
        scene.mRootNode->mMeshes[0] = 0;
        scene.mRootNode->mNumMeshes = 1;

        auto pMesh = scene.mMeshes[0];

        const auto& vertices = mesh.vertices;
        const auto& uvs = mesh.uvs;

        pMesh->mVertices = new aiVector3D[vertices.size()];
        pMesh->mNumVertices = vertices.size();

        pMesh->mTextureCoords[0] = new aiVector3D[uvs.size()];
        pMesh->mNumUVComponents[0] = uvs.size();

        for (size_t i = 0; i < vertices.size(); ++i) {
            const Vector3& v = vertices[i];
            const Vector2& uv = uvs[i];

            pMesh->mVertices[i] = aiVector3D(v.x, v.y, v.z);
            pMesh->mTextureCoords[0][i] = aiVector3D(uv.x, uv.y, 0);
        }

        const auto& indices = mesh.indices;

        pMesh->mFaces = new aiFace[indices.size() / 3];
        pMesh->mNumFaces = indices.size() / 3;

        for (size_t ti = 0; ti < indices.size() / 3; ++ti) {
            aiFace& face = pMesh->mFaces[ti];

            face.mIndices = new unsigned int[3];
            face.mNumIndices = 3;

            face.mIndices[0] = indices[ti * 3 + 0];
            face.mIndices[1] = indices[ti * 3 + 1];
            face.mIndices[2] = indices[ti * 3 + 2];
        }
    }

    Assimp::Exporter exporter;
    aiReturn res = exporter.Export(&scene, "fbx", path.string().c_str());
}