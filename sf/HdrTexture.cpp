#include "HdrTexture.h"

#include "vendor/stb_image/stb_image.h"
#include <iostream>
#include <glad\glad.h>

void HdrTexture::CreateFromFile(const std::string& filePath)
{
    stbi_set_flip_vertically_on_load(1);

    m_localBuffer = stbi_loadf(filePath.c_str(), &m_width, &m_height, &m_BPP, 0);
    if (m_localBuffer)
    {
        glGenTextures(1, &m_gl_id);
        glBindTexture(GL_TEXTURE_2D, m_gl_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, m_localBuffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(m_localBuffer);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }
}

HdrTexture::~HdrTexture()
{
    glDeleteTextures(1, &m_gl_id);
}

void HdrTexture::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_gl_id);
}

void HdrTexture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
