#pragma once

#include <glad/glad.h>
#include <string>

namespace sf {

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

		unsigned int gl_id;
		int width, height, channelCount;
		ContentType contentType;
		StorageType storageType;
		WrapMode wrapMode;
		float* floatImgBuffer = nullptr;
		unsigned char* standardImgBuffer = nullptr;
		bool needToFreeBuffer = true;

		void GetGlEnums(int channelCount, StorageType storageType, ContentType contentType, GLenum& type, int& internalFormat, GLenum& format);

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

		void CreateFromChannel(const Texture& source, int channel = 0, bool mipmap = true);

		void ComputeMipmap();
		~Texture();

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;
	};
}