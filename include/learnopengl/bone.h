#pragma once

/* Container for bone data */

#include <vector>
#include <assimp/scene.h>
#include <list>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <learnopengl/assimp_glm_helpers.h>

// 我们首先为我们的键类型创建 3 个结构。每个结构都有一个值和一个时间戳。时间戳告诉我们在动画的哪个点需要插值到它的值
struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};


// Bone 类: 从 aiNodeAnim 读取所有关键帧数据的单个骨骼。它还将根据当前动画时间在关键帧之间进行插值，即平移、缩放和旋转
class Bone {
private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

public:
    // 构造函数，从 aiNodeAnim 读取密钥并将密钥及其时间戳存储到 mPositionKeys, mRotationKeys 和 mScalingKeys
    Bone(const std::string &name, int ID, const aiNodeAnim* channel)
        :
        m_Name(name),
        m_ID(ID),
        m_LocalTransform(1.0f) {
        m_NumPositions = channel->mNumPositionKeys;

        for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex) {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            float timeStamp = channel->mPositionKeys[positionIndex].mTime;
            KeyPosition data;
            data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
            data.timeStamp = timeStamp;
            m_Positions.push_back(data);
        }

        m_NumRotations = channel->mNumRotationKeys;
        for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex) {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            KeyRotation data;
            data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            data.timeStamp = timeStamp;
            m_Rotations.push_back(data);
        }

        m_NumScalings = channel->mNumScalingKeys;
        for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex) {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            float timeStamp = channel->mScalingKeys[keyIndex].mTime;
            KeyScale data;
            data.scale = AssimpGLMHelpers::GetGLMVec(scale);
            data.timeStamp = timeStamp;
            m_Scales.push_back(data);
        }
    }

    // 主要插值过程，在每帧调用一次。调用所有键类型的相应插值函数，并组合所有最终插值结果，存储到 4x4 矩阵 m_LocalTransform中
    void Update(float animationTime) {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScaling(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    glm::mat4 GetLocalTransform() { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() { return m_ID; }

    int GetPositionIndex(float animationTime) {
        for (int index = 0; index < m_NumPositions - 1; ++index) {
            if (animationTime < m_Positions[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }

    int GetRotationIndex(float animationTime) {
        for (int index = 0; index < m_NumRotations - 1; ++index) {
            if (animationTime < m_Rotations[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }

    int GetScaleIndex(float animationTime) {
        for (int index = 0; index < m_NumScalings - 1; ++index) {
            if (animationTime < m_Scales[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }


private:

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
        float scaleFactor = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    // 平移和缩放关键点的插值函数相似
    glm::mat4 InterpolatePosition(float animationTime) {
        if (1 == m_NumPositions)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        int p0Index = GetPositionIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
                                           m_Positions[p1Index].timeStamp, animationTime);
        glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position
                                           , scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    glm::mat4 InterpolateScaling(float animationTime) {
        if (1 == m_NumScalings)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        int p0Index = GetScaleIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
                                           m_Scales[p1Index].timeStamp, animationTime);
        glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
                                        , scaleFactor);
        return glm::scale(glm::mat4(1.0f), finalScale);
    }

    // ，但对于旋转，我们使用 Slerp 在四元数之间进行插值。
    glm::mat4 InterpolateRotation(float animationTime) {
        if (1 == m_NumRotations) {
            auto rotation = glm::normalize(m_Rotations[0].orientation);
            return glm::toMat4(rotation);
        }

        int p0Index = GetRotationIndex(animationTime);
        int p1Index = p0Index + 1;
        // 比例因子，范围为 0-1 的值
        float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
                                           m_Rotations[p1Index].timeStamp, animationTime);
        // slerp 第一个参数取最后一个键，第二个参数取下一个键和第三个参数取比例因子
        // https://learnopengl-cn.github.io/img/08/01/scale_factor.png
        glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation
                                             , scaleFactor);
        finalRotation = glm::normalize(finalRotation);
        return glm::toMat4(finalRotation);

    }
};

