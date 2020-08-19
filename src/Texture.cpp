#include "Texture.h"
#include "glad/glad.h"
#include <string>
#include <iostream>

#include <stb_image.h>

void Texture::CreateFromFile(const std::string& path, Type t)
{
	m_gl_id = 0;
	m_localBuffer = nullptr;
	m_width = 0;
	m_height = 0;
	m_BPP = 0;

	stbi_set_flip_vertically_on_load(1);

	int channelCount;
	switch (t)
	{
	default:
	case Type::Albedo:
		channelCount = 4;
		break;
	case Type::Normals:
		channelCount = 3;
		break;
	case Type::Roughness:
	case Type::Metallic:
		channelCount = 1;
		break;
	}
	m_localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_BPP, channelCount);

	if (m_localBuffer == nullptr)
		std::cout << "could not load image " << path << std::endl;

	glGenTextures(1, &m_gl_id);
	glBindTexture(GL_TEXTURE_2D, m_gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	switch (t)
	{
	case Type::Albedo:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localBuffer);
		break;
	case Type::Normals:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_localBuffer);
		break;
	case Type::Roughness:
	case Type::Metallic:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, m_localBuffer);
		break;
	}

	// mipmapping
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	if (m_localBuffer)
		stbi_image_free(m_localBuffer);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_gl_id);
}

void Texture::Bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_gl_id);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}