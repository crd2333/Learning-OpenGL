#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <learnopengl/bone.h>
#include <functional>
#include <learnopengl/animdata.h>
#include <learnopengl/model_animation.h>

// AssimpNodeData 结构体：帮助我们将动画从 Assimp 提取出来
struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

// Animation 类：从 aiAnimation 读取数据并创建 Bones 的继承记录的资源
class Animation {
private:
    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
public:
    Animation() = default;

    // 两个注意点，路径是引用，模型是指针
    Animation(const std::string &animationPath, Model* model) {
        // 创建一个 Assimp::Importer 来读取动画文件，并进行一些检查
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        // 读取一般的动画数据，比如这个动画有多长，即 mDuration 和由 mTicksPerSecond 表示的动画速度
        auto animation = scene->mAnimations[0];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
        globalTransformation = globalTransformation.Inverse();
        // 调用 ReadHeirarchyData，它复制 Assimp 的 aiNode 继承权并创建 AssimpNodeData 的继承权
        ReadHierarchyData(m_RootNode, scene->mRootNode);
        // 调用一个名为 ReadMissingBones 的函数。我不得不编写这个函数，因为有时当我单独加载 FBX 模型时，它缺少一些骨骼，而我在动画文件中找到了这些缺失的骨骼。此函数读取丢失的骨骼信息，并将其信息存储在模型的 m_BoneInfoMap 中，并在 m_BoneIInfoMap 中本地保存 m_BoneIinfoMap 的引用
        ReadMissingBones(animation, *model);
    }

    ~Animation() {
    }

    Bone* FindBone(const std::string &name) {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
        [&](const Bone & Bone) {
            return Bone.GetBoneName() == name;
        }
                                );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

    inline float GetTicksPerSecond() { return m_TicksPerSecond; }
    inline float GetDuration() { return m_Duration;}
    inline const AssimpNodeData &GetRootNode() { return m_RootNode; }
    inline const std::map<std::string, BoneInfo> &GetBoneIDMap() {
        return m_BoneInfoMap;
    }

private:
    void ReadMissingBones(const aiAnimation* animation, Model &model) {
        int size = animation->mNumChannels;

        auto &boneInfoMap = model.GetBoneInfoMap(); // getting m_BoneInfoMap from Model class
        int &boneCount = model.GetBoneCount(); // getting the m_BoneCounter from Model class

        // reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++) {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones.push_back(Bone(channel->mNodeName.data,
                                   boneInfoMap[channel->mNodeName.data].id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
    }

    void ReadHierarchyData(AssimpNodeData &dest, const aiNode* src) {
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++) {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }
};

