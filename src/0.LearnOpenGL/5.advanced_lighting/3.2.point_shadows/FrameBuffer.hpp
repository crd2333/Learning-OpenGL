//
// Created by mf on 2024/12/5.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "MyTexture.hpp"

#define Err(message) do {                                                           \
    std::cout << "\33[1;31m[" << __FILE__ << ":" << __LINE__ << ", in function " << \
    __FUNCTION__ << "] " << message << "\33[0m\n";                                  \
    exit(-1);                                                                       \
} while (0)

// 帧缓冲对象的抽象封装
// 一个帧缓冲对象包含三个子部分：color buffer、depth buffer 和 stencil buffer
//     其中 color buffer 是必须的否则不完整，但可以显式声明不使用任何颜色数据进行渲染
//     后二者可以结合成 depth-stencil buffer 以节省显存
// 每种 buffer 可以是一个 texture attachment 或 renderbuffer object(RBO)
//     理论上来说都做成 texture 也是可以的，但 RBO 作为非通用数据缓冲专门被引入 OpenGL，其性能更好
//     但是 RBO 不能被采样，它是只写的，适用于需要深度和模板值用于测试的场合，如果需要采样就只能用 texture
// 一个完整的 FBO 必须：附加至少一个 buffer，至少有一个 color attachment，所有的 attachment 都是完整的（分配了内存），每个 buffer 都应该有相同的样本数

// 这里我预定义几种常见的 FBO 类型进行封装
enum FBO_TYPE {
    FBO_COLOR_TEXT_ONLY, // 只使用纹理作为附件的颜色缓冲，如《learnOpenGL-抗锯齿》小节中的 intermediateFBO
    FBO_DEPTH_TEXT_ONLY, // 只使用纹理作为附件的深度缓冲，如《learnOpenGL-阴影映射》小节中的 depthMapFBO
    FBO_DEPTH_CUBE_ONLY, // 只使用立方体纹理作为附件的深度缓冲，如《learnOpenGL-点阴影》小节中的 depthMapFBO
    FBO_CO_TEXT_DEPSTEN_RBO,             // 颜色缓冲使用纹理，深度和模板缓冲使用 RBO，如《learnOpenGL-帧缓冲》小节中的 FBO
    FBO_CO_TEXT_DEPSTEN_RBO_MULTISAMPLE, // 颜色缓冲使用纹理，深度和模板缓冲使用 RBO，且使用多重采样，如《learnOpenGL-抗锯齿》小节中的 framebuffer
};

// basic abstraction of framebuffer object
class FrameBuffer {
public:
    FrameBuffer(GLuint width, GLuint height, FBO_TYPE type) : Width(width), Height(height) {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        if (type == FBO_COLOR_TEXT_ONLY) {
            Texture2DBuffer = new TEXTURE2D_ATTACH();
            Texture2DBuffer->Generate(width, height);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture2DBuffer->ID, 0);
        } else if (type == FBO_DEPTH_TEXT_ONLY) {
            Texture2DBuffer = new TEXTURE2D_ATTACH();
            Texture2DBuffer->internal_format = GL_DEPTH_COMPONENT;
            Texture2DBuffer->format = GL_DEPTH_COMPONENT;
            Texture2DBuffer->data_type = GL_FLOAT;
            Texture2DBuffer->filter_min = GL_NEAREST;
            Texture2DBuffer->filter_max = GL_NEAREST;
            Texture2DBuffer->wrap_s = GL_CLAMP_TO_BORDER; // 设置纹理坐标裁剪到边界
            Texture2DBuffer->wrap_t = GL_CLAMP_TO_BORDER; // 让超出深度贴图的部分不会受到阴影影响
            Texture2DBuffer->Generate(width, height);
            glBindTexture(GL_TEXTURE_2D, Texture2DBuffer->ID);
            float borderColor[] = { 1.0, 1.0, 1.0, 1.0 }; // 设置边界颜色为白色，超出深度贴图的部分被设置为最大深度值 1.0
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Texture2DBuffer->ID, 0);
            glDrawBuffer(GL_NONE); // 显式声明不使用颜色数据进行渲染
            glReadBuffer(GL_NONE);
        } else if (type == FBO_DEPTH_CUBE_ONLY) {
            CubeBuffer = new CUBE_MAP_ATTACH();
            CubeBuffer->internal_format = GL_DEPTH_COMPONENT;
            CubeBuffer->format = GL_DEPTH_COMPONENT;
            CubeBuffer->data_type = GL_FLOAT;
            CubeBuffer->filter_min = GL_NEAREST;
            CubeBuffer->filter_max = GL_NEAREST;
            CubeBuffer->Generate(width, height);
            glBindTexture(GL_TEXTURE_CUBE_MAP, CubeBuffer->ID);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CubeBuffer->ID, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        } else if (type == FBO_CO_TEXT_DEPSTEN_RBO) {
            Texture2DBuffer = new TEXTURE2D_ATTACH();
            Texture2DBuffer->Generate(width, height);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture2DBuffer->ID, 0);
            glGenRenderbuffers(1, &RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            // 使用同一个 RBO，附加到帧缓冲的深度*和*模板附件上
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
        } else if (type == FBO_CO_TEXT_DEPSTEN_RBO_MULTISAMPLE) {
            Texture2DBuffer = new TEXTURE2D_ATTACH();
            Texture2DBuffer->Multisample = true;
            Texture2DBuffer->samples = 4;
            Texture2DBuffer->Generate(width, height);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, Texture2DBuffer->ID, 0);
            glGenRenderbuffers(1, &RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, Texture2DBuffer->samples, GL_DEPTH24_STENCIL8, width, height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
        } else {
            std::cout << "Framebuffer type not supported yet!" << std::endl;
            exit(-1);
        }
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete. Check framebuffers!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ~FrameBuffer() {
        glDeleteFramebuffers(1, &FBO);
        if (Texture2DBuffer) delete Texture2DBuffer;
        if (CubeBuffer) delete CubeBuffer;
        if (RBO) glDeleteRenderbuffers(1, &RBO);
    }
    void Bind() {
        glViewport(0, 0, Width, Height);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    }
    void UnBind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    // 做类似于 glBlitNamedFramebuffer 的操作
    void FromBlitTo(const FrameBuffer& draw_target, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLuint filter = GL_NEAREST) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_target.FBO);
        glBlitFramebuffer(0, 0, Width, Height, 0, 0, draw_target.Width, draw_target.Height, mask, filter);
    }
    void Clear(glm::vec4 clearColor = glm::vec4(0.0f),
               GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT) {
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(mask);
    }
    void Clear(GLbitfield mask = GL_DEPTH_BUFFER_BIT) { // cpp 只能从右到左依次省略参数，不能跳过，只能重载
        glClear(mask);
    }
    void Enable(GLbitfield mask = GL_DEPTH_TEST) {
        if (mask & GL_DEPTH_TEST) glEnable(GL_DEPTH_TEST);
        if (mask & GL_STENCIL_TEST) glEnable(GL_STENCIL_TEST);
    }
    void Disable(GLbitfield mask = GL_DEPTH_TEST) {
        if (mask & GL_DEPTH_TEST) glDisable(GL_DEPTH_TEST);
        if (mask & GL_STENCIL_TEST) glDisable(GL_STENCIL_TEST);
    }
    void BindTextureBuffer(GLint unit = 0) {
        if (!Texture2DBuffer && !CubeBuffer)
            Err("No corresponding texture buffer in this framebuffer!");
        if (Texture2DBuffer)
            Texture2DBuffer->Bind(unit);
        else
            CubeBuffer->Bind(unit);
    }
    void UnBindTextureBuffer() {
        if (!Texture2DBuffer && !CubeBuffer)
            Err("No corresponding texture buffer in this framebuffer!");
        if (Texture2DBuffer)
            Texture2DBuffer->UnBind();
        else
            CubeBuffer->UnBind();
    }

private:
    GLuint FBO; // 帧缓冲对象
    GLuint Width, Height; // 帧缓冲的宽高
    GLuint RBO; // 渲染缓冲对象
    TEXTURE2D_ATTACH* Texture2DBuffer = nullptr; // 颜色或深度纹理缓冲
    CUBE_MAP_ATTACH*  CubeBuffer = nullptr;    // 立方体纹理缓冲
};