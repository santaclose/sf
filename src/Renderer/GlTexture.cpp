#include "GlTexture.h"

#include <string>
#include <cassert>
#include <iostream>

void sf::GlTexture::Create(uint32_t width, uint32_t height, int channelCount, DataType storageDataType, WrapMode wrapMode, bool mipmap)
{
	if (this->isInitialized)
		glDeleteTextures(1, &this->gl_id);

	this->isInitialized = true;
	this->width = width;
	this->height = height;
	this->storageDataType = storageDataType;
	this->wrapMode = wrapMode;

	int internalFormat;
	GLenum type, format;
	DeduceGlTextureEnums(channelCount, this->storageDataType, type, internalFormat, format);

	glGenTextures(1, &this->gl_id);
	glBindTexture(GL_TEXTURE_2D, this->gl_id);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, format, type, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

}

void sf::GlTexture::CreateFromBitmap(const Bitmap& bitmap, WrapMode wrapMode, bool mipmap, int internalFormat)
{
	if (this->isInitialized)
		glDeleteTextures(1, &this->gl_id);

	this->isInitialized = true;
	this->height = bitmap.height;
	this->width = bitmap.width;
	this->channelCount = bitmap.channelCount;
	this->wrapMode = wrapMode;
	this->storageDataType = bitmap.dataType;

	glGenTextures(1, &this->gl_id);
	glBindTexture(GL_TEXTURE_2D, this->gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	int deducedInternalFormat;
	GLenum type, format;
	DeduceGlTextureEnums(this->channelCount, this->storageDataType, type, deducedInternalFormat, format);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat == -1 ? deducedInternalFormat : internalFormat, this->width, this->height, 0, format, type, bitmap.buffer);

	if (mipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void sf::GlTexture::ComputeMipmap()
{
	glBindTexture(GL_TEXTURE_2D, this->gl_id);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void sf::GlTexture::Delete()
{
	if (this->gl_id != 0)
		glDeleteTextures(1, &this->gl_id);
	this->gl_id = 0;
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

void sf::DeduceGlTextureEnums(int channelCount, DataType storageDataType, GLenum& type, int& internalFormat, GLenum& format)
{
	switch (storageDataType)
	{
	case DataType::u8:
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
	case DataType::u16:
		type = GL_UNSIGNED_SHORT;
		switch (channelCount)
		{
		case 1:
			format = GL_RED;
			internalFormat = GL_R16;
			break;
		case 2:
			format = GL_RG;
			internalFormat = GL_RG16;
			break;
		case 3:
			format = GL_RGB;
			internalFormat = GL_RGB16;
			break;
		case 4:
			format = GL_RGBA;
			internalFormat = GL_RGBA16;
			break;
		}
		break;
	case DataType::f16:
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
	case DataType::f32:
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
	default:
		assert(!"Data type not handled");
		break;
	}
}