#include "Texture.h"
#include "GltfController.h"
#include <string>
#include <iostream>

#include <stb_image.h>


void Texture::GetGlEnums(int channelCount, StorageType storageType, ContentType contentType, GLenum& type, int& internalFormat, GLenum& format)
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
			internalFormat = contentType == ContentType::Color ? GL_SRGB : GL_RGB8;
			break;
		case 4:
			format = GL_RGBA;
			internalFormat = contentType == ContentType::Color ? GL_SRGB_ALPHA : GL_RGBA8;
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

void Texture::Create(unsigned int width, unsigned int height, int channelCount,	ContentType contentType, StorageType storageType, WrapMode wrapMode, bool mipmap)
{
	m_width = width;
	m_height = height;
	m_contentType = contentType;
	m_storageType = storageType;
	m_wrapMode = wrapMode;

	int internalFormat;
	GLenum type, format;
	GetGlEnums(channelCount, m_storageType, m_contentType, type, internalFormat, format);

	glGenTextures(1, &m_gl_id);
	glBindTexture(GL_TEXTURE_2D, m_gl_id);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, type, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

}

void Texture::CreateFromFile(const std::string& path, int channelCount, ContentType contentType, StorageType storageType, WrapMode wrapMode, bool mipmap, bool flipVertically)
{
	m_contentType = contentType;
	m_storageType = storageType;
	m_wrapMode = wrapMode;

	float* floatImgBuffer = nullptr;
	unsigned char* standardImgBuffer = nullptr;

	stbi_set_flip_vertically_on_load(flipVertically);

	bool isFloatTexture = m_storageType == StorageType::Float16 || m_storageType == StorageType::Float32;
	if (isFloatTexture)
	{
		floatImgBuffer = stbi_loadf(path.c_str(), &m_width, &m_height, &m_channelCount, channelCount);
		if (floatImgBuffer == nullptr)
			std::cout << "[Texture] Could not load image " << path << std::endl;
	}
	else
	{
		standardImgBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_channelCount, channelCount);
		if (standardImgBuffer == nullptr)
			std::cout << "[Texture] Could not load image " << path << std::endl;
	}
	m_channelCount = channelCount;

	glGenTextures(1, &m_gl_id);
	glBindTexture(GL_TEXTURE_2D, m_gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);


	int internalFormat;
	GLenum type, format;
	GetGlEnums(m_channelCount, m_storageType, m_contentType, type, internalFormat, format);

	void* dataPointer = isFloatTexture ? (void*)floatImgBuffer : (void*)standardImgBuffer;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, type, dataPointer);

	if (mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	if (standardImgBuffer)
		stbi_image_free(standardImgBuffer);
	if (floatImgBuffer)
		stbi_image_free(floatImgBuffer);
}

void Texture::CreateFromGltf(unsigned int gltfID, unsigned int textureIndex)
{
	GltfController::Texture(gltfID, textureIndex, m_gl_id, m_width, m_height);
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