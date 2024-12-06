//
// Created by mf on 2024/12/5.
//

#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <vector>

class Texture2D {
public:
    GLuint ID = 0; // 每个纹理的唯一标识
    // image infomation
    GLint  width, height, nrChannels; // 图像的宽度、高度和通道数（由读取时 stb_image 返回）
    bool   flip_y;                    // 是否翻转图像的 y 轴
    GLuint image_format;              // 要读取的图像的格式
    // texture infomation
    GLuint internal_format; // 读取后纹理对象的格式
    GLuint data_type;       // 读取后纹理的数据格式，一般都是 GL_UNSIGNED_BYTE
    GLuint wrap_s;          // Wrapping mode on S axis (X axis)，一般是 GL_REPEAT 或 GL_CLAMP_TO_EDGE
    GLuint wrap_t;          // Wrapping mode on T axis
    GLuint filter_min;      // Filtering mode if texture pixels < screen pixels，一般是 GL_LINEAR 或 GL_NEAREST
    GLuint filter_max;      // Filtering mode if texture pixels > screen pixels
    bool   mipmap;          // 是否生成 mipmap

    // Constructor（设置默认纹理参数）
    Texture2D() : flip_y(false), image_format(GL_RGB), internal_format(GL_RGB), data_type(GL_UNSIGNED_BYTE),
                  wrap_s(GL_REPEAT), wrap_t(GL_REPEAT), filter_min(GL_LINEAR), filter_max(GL_LINEAR), mipmap(true) {
        glGenTextures(1, &ID);
    }
    ~Texture2D() {
        glDeleteTextures(1, &ID);
    }
    // 根据设置的参数，利用 stb_image 读取图像数据并生成纹理
    void Generate(std::string path) {
        glBindTexture(GL_TEXTURE_2D, ID);

        // read image data from file
        stbi_set_flip_vertically_on_load(flip_y);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
        if (__builtin_expect(!!data, 1)) {
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, image_format, data_type, data);
            if (mipmap) glGenerateMipmap(GL_TEXTURE_2D); // 生成 mipmap
        } else {
            std::cout << "Failed to load texture for " << path << std::endl;
        }
        stbi_image_free(data);

        // set the texture filtering and wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_max);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    }
    // Binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind(GLint unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
    void UnBind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

// 用于立方体贴图的纹理（如天空盒）
class CUBE_MAP {
public:
    GLuint ID;
    // image infomation
    int    width, height, nrChannels;
    bool   flip_y;
    GLuint image_format;
    // texture infomation
    GLuint internal_format;
    GLuint data_type;
    GLuint wrap_s;
    GLuint wrap_t;
    GLuint wrap_r;
    GLuint filter_min;
    GLuint filter_max;

    // Constructor（设置默认纹理参数）
    CUBE_MAP() : flip_y(false), image_format(GL_RGB), internal_format(GL_RGB), data_type(GL_UNSIGNED_BYTE),
                 wrap_s(GL_CLAMP_TO_EDGE), wrap_t(GL_CLAMP_TO_EDGE), wrap_r(GL_CLAMP_TO_EDGE),
                 filter_min(GL_LINEAR), filter_max(GL_LINEAR) {
        glGenTextures(1, &ID);
    }
    ~CUBE_MAP() {
        glDeleteTextures(1, &ID);
    }
    // 根据设置的参数，利用 stb_image 读取图像数据并生成纹理
    void Generate(std::vector<std::string> paths) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

        // read image data from file
        // order: +X(right), -X(left), +Y(top), -Y(bottom), +Z(front), -Z(back)
        stbi_set_flip_vertically_on_load(flip_y);
        for (unsigned int i = 0; i < paths.size(); i++) {
            unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nrChannels, 0);
            if (__builtin_expect(!!data, 1))
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width, height, 0, image_format, data_type, data);
            else
                std::cout << "Failed to load cubemap texture for " << paths[i] << std::endl;
            stbi_image_free(data);
        }

        // set the texture filtering and wrapping parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter_min);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter_max);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r);
    }
    // Binds the texture as the current active GL_TEXTURE_CUBE_MAP texture object
    void Bind(GLint unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    }
    void UnBind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};

// 作为帧缓冲附件的纹理
class TEXTURE2D_ATTACH {
public:
    GLuint ID;
    // texture infomation
    GLuint internal_format;
    GLuint format;
    GLuint data_type;
    GLuint wrap_s;
    GLuint wrap_t;
    GLuint filter_min;
    GLuint filter_max;
    bool   Multisample;
    int samples = -1;

    // Constructor（设置默认纹理参数）
    TEXTURE2D_ATTACH() : internal_format(GL_RGB), format(GL_RGB), data_type(GL_UNSIGNED_BYTE),
                         wrap_s(GL_CLAMP_TO_EDGE), wrap_t(GL_CLAMP_TO_EDGE),
                         filter_min(GL_LINEAR), filter_max(GL_LINEAR), Multisample(false) {
        glGenTextures(1, &ID);
    }
    ~TEXTURE2D_ATTACH() {
        glDeleteTextures(1, &ID);
    }
    // 生成一个空的纹理作为附件（只分配内存而没有填充，用于帧缓冲 framebuffer）
    void Generate(int width, int height) {
        glBindTexture(Multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, ID);
        if (Multisample)
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internal_format, width, height, GL_TRUE);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, data_type, NULL);
        // set the texture filtering and wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_min);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_max);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    }
    // Binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind(GLint unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
    void UnBind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

// 作为帧缓冲附件的立方体贴图纹理
class CUBE_MAP_ATTACH {
public:
    GLuint ID;
    // texture infomation
    GLuint internal_format;
    GLuint format;
    GLuint data_type;
    GLuint wrap_s;
    GLuint wrap_t;
    GLuint wrap_r;
    GLuint filter_min;
    GLuint filter_max;

    // Constructor（设置默认纹理参数）
    CUBE_MAP_ATTACH() : internal_format(GL_RGB), format(GL_RGB), data_type(GL_UNSIGNED_BYTE),
                        wrap_s(GL_CLAMP_TO_EDGE), wrap_t(GL_CLAMP_TO_EDGE), wrap_r(GL_CLAMP_TO_EDGE),
                        filter_min(GL_LINEAR), filter_max(GL_LINEAR) {
        glGenTextures(1, &ID);
    }
    ~CUBE_MAP_ATTACH() {
        glDeleteTextures(1, &ID);
    }
    // 生成一个空的立方体贴图纹理作为附件（只分配内存而没有填充，用于帧缓冲 framebuffer）
    void Generate(int width, int height) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
        for (unsigned int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, width, height, 0, format, data_type, NULL);
        // set the texture filtering and wrapping parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filter_min);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filter_max);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r);
    }
    // Binds the texture as the current active GL_TEXTURE_CUBE_MAP texture object
    void Bind(GLint unit = 0) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    }
    void UnBind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};