#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int main() {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("Resources/human/walk.gltf", aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
    if (!scene) {
        std::cout << "Failed to load scene" << std::endl;
        return 1;
    }
    std::cout << "Num Meshes: " << scene->mNumMeshes << std::endl;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        std::cout << "Mesh " << i << " Num Bones: " << scene->mMeshes[i]->mNumBones << std::endl;
    }
    return 0;
}
