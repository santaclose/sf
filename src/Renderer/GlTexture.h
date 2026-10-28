#pragma once

#include <glad/glad.h>
#include <string>

#include <Bitmap.h>

namespace sf {

	class GlTexture
	{
	public:
		enum WrapMode {
			Repeat, ClampToEdge
		};

		bool isInitialized = false;
		uint32_t gl_id;
		int width, height, depth, channelCount;
		DataType storageDataType;
		WrapMode wrapMode;

		uint32_t GetTarget() const { return depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D; }

		void Create(
			uint32_t width,
			uint32_t height,
			uint32_t depth = 1,
			int channelCount = 3,
			DataType storageDataType = DataType::u8,
			WrapMode wrapMode = WrapMode::Repeat,
			bool mipmap = true);

		void CreateFromBitmap(
			const Bitmap& bitmap,
			WrapMode wrapMode =
			WrapMode::Repeat,
			bool mipmap = true,
			int internalFormat = -1);

		void Delete();
		GlTexture() = default;
		~GlTexture() = default;

		void ComputeMipmap();

		void Bind(uint32_t slot = 0) const;
		void Unbind() const;
	};

	void DeduceGlTextureEnums(int channelCount, DataType storageDataType, GLenum& type, int& internalFormat, GLenum& format);
}