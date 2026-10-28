#include "GlTexture.h"

#include <string>
#include <cassert>
#include <iostream>

void sf::GlTexture::Create(uint32_t width, uint32_t height, uint32_t depth, int channelCount, DataType storageDataType, WrapMode wrapMode, bool mipmap)
{
	if (this->isInitialized)
		glDeleteTextures(1, &this->gl_id);

	this->isInitialized = true;
	this->width = width;
	this->height = height;
	this->depth = depth;
	this->storageDataType = storageDataType;
	this->wrapMode = wrapMode;

	uint32_t texTarget = GetTarget();

	int internalFormat;
	GLenum type, format;
	DeduceGlTextureEnums(channelCount, this->storageDataType, type, internalFormat, format);

	glGenTextures(1, &this->gl_id);
	glBindTexture(texTarget, this->gl_id);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (texTarget == GL_TEXTURE_2D)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, format, type, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, this->width, this->height, this->depth, 0, format, type, nullptr);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	}

}

void sf::GlTexture::CreateFromBitmap(const Bitmap& bitmap, WrapMode wrapMode, bool mipmap, int internalFormat)
{
	if (this->isInitialized)
		glDeleteTextures(1, &this->gl_id);

	this->isInitialized = true;
	this->height = bitmap.height;
	this->width = bitmap.width;
	this->depth = bitmap.depth;
	this->channelCount = bitmap.channelCount;
	this->wrapMode = wrapMode;
	this->storageDataType = bitmap.dataType;

	uint32_t texTarget = GetTarget();

	int deducedInternalFormat;
	GLenum type, format;
	DeduceGlTextureEnums(this->channelCount, this->storageDataType, type, deducedInternalFormat, format);

	glGenTextures(1, &this->gl_id);
	glBindTexture(texTarget, this->gl_id);

	if (texTarget == GL_TEXTURE_2D)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat == -1 ? deducedInternalFormat : internalFormat, this->width, this->height, 0, format, type, bitmap.buffer);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, this->wrapMode == WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage3D(GL_TEXTURE_3D, 0, internalFormat == -1 ? deducedInternalFormat : internalFormat, this->width, this->height, this->depth, 0, format, type, bitmap.buffer);
	}

	if (mipmap)
		glGenerateMipmap(texTarget);

	glBindTexture(texTarget, 0);
}

void sf::GlTexture::ComputeMipmap()
{
	glBindTexture(GetTarget(), this->gl_id);
	glGenerateMipmap(GetTarget());
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
	glBindTexture(GetTarget(), this->gl_id);
}

void sf::GlTexture::Unbind() const
{
	glBindTexture(GetTarget(), 0);
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