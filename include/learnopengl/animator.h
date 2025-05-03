#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <learnopengl/animation.h>
#include <learnopengl/bone.h>

// Animator 类：读取 AssimpNodeData 的继承方法，以递归方式插入所有骨骼，然后为我们准备所需的最终骨骼转换矩阵
class Animator {
private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
public:
    Animator(Animation* animation) {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;

        m_FinalBoneMatrices.reserve(100);

        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    // 以 m_TicksPerSecond 的速率推进 m_CurrentTime，然后调用 CalculateBoneTransform 函数
    void UpdateAnimation(float dt) {
        m_DeltaTime = dt;
        if (m_CurrentAnimation) {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            // 两个参数，第一个是 m_CurrentAnimation 的 m_RootNode，第二个是作为 parentTransform 传递的身份矩阵
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    void PlayAnimation(Animation* pAnimation) {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
    }

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        // 通过在 animation 类的 m_Bones 数组中查找 nodeName 对应的 m_RootNodes 骨骼来检查该骨骼是否参与该动画
        Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

        // 如果找到骨骼，则调用 bone.Update() 函数，对所有骨骼进行插值，并将局部骨骼变换矩阵返回到 nodeTransform
        if (Bone) {
            Bone->Update(m_CurrentTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        // 但这是局部空间矩阵，如果在着色器中传递，将围绕原点移动骨骼。因此，我们将这个 nodeTransform 与 parentTransform 相乘，并将结果存储在 globalTransformation 中
        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        // 这就足够了，但顶点仍在默认模型空间中。我们在 m_BoneInfoMap 中找到偏移矩阵，然后将其与 globalTransfromMatrix 相乘。我们还将获得 id 索引，该索引将用于写入该骨骼到 m_FinalBoneMatrices 的最终转换
        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }

    std::vector<glm::mat4> GetFinalBoneMatrices() {
        return m_FinalBoneMatrices;
    }
};
