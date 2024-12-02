#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glad/glad.h>
#include <iostream>

class Texture {
public:
    Texture(std::string_view path) {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        stbi_set_flip_vertically_on_load(true); // tell stb/stb_image.h to flip loaded texture's on the y-axis.

        unsigned char* data = stbi_load(path.data(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
            std::cout << "Failed to load texture" << std::endl;
        stbi_image_free(data);
    }
    ~Texture() {
        glDeleteTextures(1, &textureID);
    }

    void Bind(unsigned int slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    void Unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    unsigned int GetTextureID() const { return textureID; }
private:
    unsigned int textureID;
    std::string_view path;
    int width, height, nrChannels;
};
