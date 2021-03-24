#pragma once

#include "glad/glad.h"
#include <string>

class Texture
{
public:
	enum ContentType {
		Color, NonColor
	};
	enum StorageType {
		UnsignedByte, Float16, Float32
	};
	enum WrapMode {
		Repeat, ClampToEdge
	};

private:
	unsigned int m_gl_id;
	int m_width, m_height, m_channelCount;
	ContentType m_contentType;
	StorageType m_storageType;
	WrapMode m_wrapMode;

	void GetGlEnums(int channelCount, StorageType storageType, ContentType contentType, GLenum& type, int& internalFormat, GLenum& format);

public:
	void Create(unsigned int width, unsigned int height,
		int channelCount = 3,
		ContentType contentType = ContentType::NonColor,
		StorageType storageType = StorageType::UnsignedByte,
		WrapMode wrapMode = WrapMode::Repeat,
		bool mipmap = true);

	void CreateFromFile(const std::string& path,
		int channelCount = 0,
		ContentType contentType = ContentType::NonColor,
		StorageType storageType = StorageType::UnsignedByte,
		WrapMode wrapMode = WrapMode::Repeat,
		bool mipmap = true,
		bool flipVertically = true);

	void CreateFromGltf(unsigned int gltfID, unsigned int textureIndex);
	void ComputeMipmap();
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline unsigned int GlId() const { return m_gl_id; }
	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
};