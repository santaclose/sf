#include "GlTexture.h"

#include <string>
#include <iostream>
#include <stb_image.h>

#include <Importer/GltfImporter.h>

void sf::GlTexture::GetGlEnums(int channelCount, StorageType storageType, GLenum& type, int& internalFormat, GLenum& format)
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

void sf::GlTexture::Create(uint32_t width, uint32_t height, int channelCount, StorageType storageType, WrapMode wrapMode, bool mipmap)
{
	this->width = width;
	this->height = height;
	this->storageType = storageType;
	this->wrapMode = wrapMode;

	int internalFormat;
	GLenum type, format;
	GetGlEnums(channelCount, this->storageType, type, internalFormat, format);

	glGenTextures(1, &this->gl_id);
	glBindTexture(GL_TEXTURE_2D, this->gl_id);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, format, type, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

}

void sf::GlTexture::CreateFromFile(const std::string& path, int channelCount, StorageType storageType, WrapMode wrapMode, bool mipmap, bool flipVertically)
{
	this->storageType = storageType;
	this->wrapMode = wrapMode;

	stbi_set_flip_vertically_on_load(flipVertically);

	bool isFloatTexture = this->storageType == StorageType::Float16 || this->storageType == StorageType::Float32;
	if (isFloatTexture)
	{
		floatImgBuffer = stbi_loadf(path.c_str(), &this->width, &this->height, &this->channelCount, channelCount);
		if (floatImgBuffer == nullptr)
			std::cout << "[GlTexture] Could not load image " << path << std::endl;
	}
	else
	{
		standardImgBuffer = stbi_load(path.c_str(), &this->width, &this->height, &this->channelCount, channelCount);
		if (standardImgBuffer == nullptr)
			std::cout << "[GlTexture] Could not load image " << path << std::endl;
	}
	this->channelCount = channelCount;

	glGenTextures(1, &this->gl_id);
	glBindTexture(GL_TEXTURE_2D, this->gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	int internalFormat;
	GLenum type, format;
	GetGlEnums(this->channelCount, this->storageType, type, internalFormat, format);

	void* dataPointer = isFloatTexture ? (void*)floatImgBuffer : (void*)standardImgBuffer;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, format, type, dataPointer);

	if (mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void sf::GlTexture::CreateFromBitmap(const Bitmap& bitmap, WrapMode wrapMode, bool mipmap)
{
	this->height = bitmap.height;
	this->width = bitmap.width;
	this->channelCount = bitmap.channelCount;
	this->wrapMode = wrapMode;
	switch (bitmap.dataType)
	{
	case DataType::u8:
		this->storageType = StorageType::UnsignedByte;
		break;
	case DataType::f16:
		this->storageType = StorageType::Float16;
		break;
	case DataType::f32:
		this->storageType = StorageType::Float32;
		break;
	default:
		std::cout << "[GLTexture] Can't create texture from this bitmap\n";
	}

	glGenTextures(1, &this->gl_id);
	glBindTexture(GL_TEXTURE_2D, this->gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	int internalFormat;
	GLenum type, format;
	GetGlEnums(this->channelCount, this->storageType, type, internalFormat, format);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, format, type, bitmap.buffer);

	if (mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void sf::GlTexture::CreateFromChannel(const GlTexture& source, int channel, bool mipmap)
{
	this->height = source.height;
	this->width = source.width;
	this->channelCount = 1;
	this->storageType = source.storageType;
	this->wrapMode = source.wrapMode;

	if (this->storageType == StorageType::UnsignedByte)
	{
		this->standardImgBuffer = new unsigned char[this->width * this->height];
		for (int i = 0; i < this->width * this->height; i++)
			this->standardImgBuffer[i] = source.standardImgBuffer[i * source.channelCount + channel];
	}
	else
	{
		this->floatImgBuffer = new float[this->width * this->height];
		for (int i = 0; i < this->width * this->height; i++)
			this->floatImgBuffer[i] = source.floatImgBuffer[i * source.channelCount + channel];
	}

	glGenTextures(1, &this->gl_id);
	glBindTexture(GL_TEXTURE_2D, this->gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);


	int internalFormat;
	GLenum type, format;
	GetGlEnums(this->channelCount, this->storageType, type, internalFormat, format);

	void* dataPointer = this->storageType != StorageType::UnsignedByte ? (void*)floatImgBuffer : (void*)standardImgBuffer;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, format, type, dataPointer);

	if (mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void sf::GlTexture::ComputeMipmap()
{
	glBindTexture(GL_TEXTURE_2D, this->gl_id);
	glGenerateMipmap(GL_TEXTURE_2D);
}

sf::GlTexture::~GlTexture()
{
	glDeleteTextures(1, &this->gl_id);

	if (standardImgBuffer != nullptr && needToFreeBuffer)
		stbi_image_free(standardImgBuffer);
	if (floatImgBuffer != nullptr && needToFreeBuffer)
		stbi_image_free(floatImgBuffer);
}

void sf::GlTexture::Bind(uint32_t slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, this->gl_id);
}

void sf::GlTexture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}