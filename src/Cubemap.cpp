#include "Cubemap.h"
#include "Shader.h"
#include "ComputeShader.h"
#include "glad/glad.h"
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Cubemap::CreateFromFiles(const std::vector<std::string>& files, bool isHdr)
{
    stbi_set_flip_vertically_on_load(0);

    m_isHdr = isHdr;
    glGenTextures(1, &m_gl_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);

    int width, height, nrChannels;
    if (!isHdr)
    {
        for (unsigned int i = 0; i < files.size(); i++)
        {
            unsigned char* data = stbi_load(files[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                std::cout << "Cubemap texture loaded successfully: " << files[i] << std::endl;
            }
            else
                std::cout << "Cubemap texture failed to load at path: " << files[i] << std::endl;
            stbi_image_free(data);
        }
    }
    else
    {
        for (unsigned int i = 0; i < files.size(); i++)
        {
            float* data = stbi_loadf(files[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
                std::cout << "Cubemap texture loaded successfully: " << files[i] << std::endl;
            }
            else
                std::cout << "Cubemap texture failed to load at path: " << files[i] << std::endl;
            stbi_image_free(data);
        }
    }


    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

/*
    "_0.jpg",
    "_1.jpg",
    "_2.jpg",
    "_3.jpg",
    "_4.jpg",
    "_5.jpg"
*/
void Cubemap::CreateFromFiles(const std::string& name, const std::string& extension, bool isHdr)
{
    std::vector<std::string> files;
    files.resize(6);
    for (int i = 0; i < 6; i++)
        files[i] = name + "_" + std::to_string(i) + extension;

    CreateFromFiles(files, isHdr);
}

bool Cubemap::IsHDR()
{
    return m_isHdr;
}

Cubemap::~Cubemap()
{
    glDeleteTextures(1, &m_gl_id);
}

void Cubemap::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_gl_id);
}

void Cubemap::Unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
