#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#ifndef STB_IMAGE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <learnopengl/mesh.h>
#include <learnopengl/shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <learnopengl/assimp_glm_helpers.h>
#include <learnopengl/animdata.h>

using namespace std;

// 相对 model.h 增加了骨骼和动画信息
class Model {
public:
    // model data
    vector<Texture> textures_loaded;    // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(string const &path, bool gamma = false) : gammaCorrection(gamma) {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader &shader) {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    auto &GetBoneInfoMap() { return m_BoneInfoMap; }
    int &GetBoneCount() { return m_BoneCounter; }


private:

    // 声明一个映射和一个计数器，一旦我们读取到一个新的骨骼，它就会增加
    std::map<string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path) {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { // if is Not Zero
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene) {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene);

    }

    void SetVertexBoneDataToDefault(Vertex &vertex) { // 将 m_BoneID 和 m_Weights 设置为默认值
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }


    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // 每个 aiMesh 都包含与 aiMesh 关联的所有 aiBone
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            SetVertexBoneDataToDefault(vertex);
            vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
            vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            } else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        ExtractBoneWeightForVertices(vertices, mesh, scene);

        return Mesh(vertices, indices, textures);
    }

    void SetVertexBoneData(Vertex &vertex, int boneID, float weight) {
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
            if (vertex.m_BoneIDs[i] < 0) {
                vertex.m_Weights[i] = weight;
                vertex.m_BoneIDs[i] = boneID;
                break;
            }
        }
    }

    void ExtractBoneWeightForVertices(std::vector<Vertex> &vertices, aiMesh* mesh, const aiScene* scene) {
        auto &boneInfoMap = m_BoneInfoMap;
        int &boneCount = m_BoneCounter;

        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) { // 为每个 aiBone 运行 for 循环
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            // 检查该骨骼是否已存在于 m_BoneInfoMap 中
            if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
                // 如果我们找不到它，那么它被认为是一块新骨骼
                // 我们创建一个带有 id 的新 BoneInfo
                BoneInfo newBoneInfo;
                newBoneInfo.id = boneCount;
                // 并将其关联的 mOffsetMatrix 存储到它
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                boneInfoMap[boneName] = newBoneInfo; // 然后我们将这个新 BoneIInfo 存储在 m_BoneInfoMap 中
                // 然后我们递增 m_BoneCounter 计数器，为下一块骨骼建一个 id
                boneID = boneCount;
                boneCount++;
            } else // 如果找到，那么这意味着该骨骼会影响超出其范围的网格顶点。所以我们取它的 Id，进一步了解它会影响哪些顶点
                boneID = boneInfoMap[boneName].id;
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            // 我们已经提取了骨骼的 offsetMatrix，现在我们将简单地迭代其 aiVertexWeightarray
            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
                // 提取将受此骨骼影响的所有顶点索引及其各自的权重
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                // 并调用 SetVertexBoneData 以使用提取的信息填充 Vertex.boneIds和 Vertex.weights
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }


    unsigned int TextureFromFile(const char* path, const string &directory, bool gamma = false) {
        string filename = string(path);
        // filename = directory + '/' + filename;
        filename = directory + "/../" + '/' + filename; // 自己加的

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        } else {
            std::cout << "Texture failed to load at filename: " << filename << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip) {
                // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        return textures;
    }
};



#endif
