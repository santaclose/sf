#pragma once

#include <glad/glad.h>
#include <string>

#include <Bitmap.h>

namespace sf {

	class GlTexture
	{
	public:
		enum StorageType {
			UnsignedByte, Float16, Float32
		};
		enum WrapMode {
			Repeat, ClampToEdge
		};

		unsigned int gl_id;
		int width, height, channelCount;
		StorageType storageType;
		WrapMode wrapMode;
		float* floatImgBuffer = nullptr;
		unsigned char* standardImgBuffer = nullptr;
		bool needToFreeBuffer = true;

		void GetGlEnums(int channelCount, StorageType storageType, GLenum& type, int& internalFormat, GLenum& format);

		void Create(unsigned int width, unsigned int height,
			int channelCount = 3,
			StorageType storageType = StorageType::UnsignedByte,
			WrapMode wrapMode = WrapMode::Repeat,
			bool mipmap = true);

		void CreateFromFile(const std::string& path,
			int channelCount = 0,
			StorageType storageType = StorageType::UnsignedByte,
			WrapMode wrapMode = WrapMode::Repeat,
			bool mipmap = true,
			bool flipVertically = true);

		void CreateFromBitmap(const Bitmap& bitmap, WrapMode wrapMode = WrapMode::Repeat, bool mipmap = true);

		void CreateFromChannel(const GlTexture& source, int channel = 0, bool mipmap = true);

		void ComputeMipmap();
		~GlTexture();

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;
	};
}