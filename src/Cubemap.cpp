#include "Cubemap.h"
#include "Shader.h"
#include "ComputeShader.h"
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void sf::Cubemap::GetGlEnums(int channelCount, StorageType storageType, GLenum& type, int& internalFormat, GLenum& format)
{
    switch (storageType)
    {
    case StorageType::UnsignedByte:
        type = GL_UNSIGNED_BYTE;
        switch (channelCount)
        {
        case 1:
            format = GL_RED;
            internalFormat = GL_R8;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG8;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB8;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;
        }
        break;
    case StorageType::Float16:
        type = GL_FLOAT;
        switch (channelCount)
        {
        case 1:
            format = GL_RED;
            internalFormat = GL_R16F;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG16F;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB16F;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA16F;
            break;
        }
        break;
    case StorageType::Float32:
        type = GL_FLOAT;
        switch (channelCount)
        {
        case 1:
            format = GL_RED;
            internalFormat = GL_R32F;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG32F;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB32F;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA32F;
            break;
        }
        break;
    }
}

void sf::Cubemap::Create(unsigned int size, int channelCount, StorageType storageType, bool mipmap)
{
    m_size = size;
    m_storageType = storageType;


    int internalFormat;
    GLenum type, format;
    GetGlEnums(channelCount, m_storageType, type, internalFormat, format);

    glGenTextures(1, &m_gl_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, size, size, 0, format, type, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void sf::Cubemap::CreateFromFiles(const std::vector<std::string>& files, int channelCount, StorageType storageType, bool mipmap)
{
    m_storageType = storageType;

    stbi_set_flip_vertically_on_load(0);

    bool isHdr = storageType == StorageType::Float16 || storageType == StorageType::Float32;
    isHdr = stbi_is_hdr(files[0].c_str());

    glGenTextures(1, &m_gl_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);

    int width, height, nrChannels;
    if (!isHdr)
    {
        for (unsigned int i = 0; i < files.size(); i++)
        {
            unsigned char* data = stbi_load(files[i].c_str(), &width, &height, &nrChannels, 0);
            m_size = width;
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                std::cout << "[Cubemap] Cubemap texture loaded successfully: " << files[i] << std::endl;
            }
            else
                std::cout << "[Cubemap] Cubemap texture failed to load at path: " << files[i] << std::endl;

            if (data)
                stbi_image_free(data);
        }
    }
    else
    {
        for (unsigned int i = 0; i < files.size(); i++)
        {
            float* data = stbi_loadf(files[i].c_str(), &width, &height, &nrChannels, 0);
            m_size = width;
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
                std::cout << "[Cubemap] Cubemap texture loaded successfully: " << files[i] << std::endl;
            }
            else
                std::cout << "[Cubemap] Cubemap texture failed to load at path: " << files[i] << std::endl;

            if (data)
                stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if (mipmap)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

/*
    "_0.jpg",
    "_1.jpg",
    "_2.jpg",
    "_3.jpg",
    "_4.jpg",
    "_5.jpg"
*/
void sf::Cubemap::CreateFromFiles(const std::string& name, const std::string& extension, int channelCount, StorageType storageType, bool mipmap)
{
    std::vector<std::string> files;
    files.resize(6);
    for (int i = 0; i < 6; i++)
        files[i] = name + "_" + std::to_string(i) + extension;

    CreateFromFiles(files, channelCount, storageType, mipmap);
}

void sf::Cubemap::ComputeMipmap()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

sf::Cubemap::~Cubemap()
{
    glDeleteTextures(1, &m_gl_id);
}

void sf::Cubemap::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);
}

void sf::Cubemap::Unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
