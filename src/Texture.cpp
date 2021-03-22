#include "Texture.h"
#include "glad/glad.h"
#include "GltfController.h"
#include <string>
#include <iostream>

#include <stb_image.h>

void Texture::CreateFromGltf(unsigned int gltfID, unsigned int textureIndex)
{
	GltfController::Texture(gltfID, textureIndex, m_gl_id, m_width, m_height);
}

void Texture::CreateFromFile(const std::string& path, Type type, bool mipmap)
{
	m_type = type;

	float* floatImgBuffer = nullptr;
	unsigned char* standardImgBuffer = nullptr;
	int m_BPP = 0;

	m_gl_id = 0;
	m_width = 0;
	m_height = 0;

	stbi_set_flip_vertically_on_load(1);

	int channelCount;
	switch (m_type)
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

	if (m_type == Type::HDR)
	{
		floatImgBuffer = stbi_loadf(path.c_str(), &m_width, &m_height, &m_BPP, 0);
		if (floatImgBuffer == nullptr)
			std::cout << "[Texture] Could not load image " << path << std::endl;
	}
	else
	{
		standardImgBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_BPP, channelCount);
		if (standardImgBuffer == nullptr)
			std::cout << "[Texture] Could not load image " << path << std::endl;
	}

	glGenTextures(1, &m_gl_id);
	glBindTexture(GL_TEXTURE_2D, m_gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (m_type == Type::HDR)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	switch (m_type)
	{
	case Type::Albedo:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, standardImgBuffer);
		break;
	case Type::Normals:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, standardImgBuffer);
		break;
	case Type::Roughness:
	case Type::Metallic:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, standardImgBuffer);
		break;
	case Type::HDR:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGB, GL_FLOAT, floatImgBuffer);
		break;
	}

	if (mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	if (standardImgBuffer)
		stbi_image_free(standardImgBuffer);
	if (floatImgBuffer)
		stbi_image_free(floatImgBuffer);
}

void Texture::ComputeMipmap()
{
	glBindTexture(GL_TEXTURE_2D, m_gl_id);
	glGenerateMipmap(GL_TEXTURE_2D);
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