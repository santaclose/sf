#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace sf {

	class GlCubemap
	{
	public:
		enum StorageType {
			UnsignedByte, Float16, Float32
		};

	private:
		unsigned int m_gl_id;
		int m_size;
		StorageType m_storageType;

		void GetGlEnums(int channelCount, StorageType storageType, GLenum& type, int& internalFormat, GLenum& format);

	public:
		void Create(
			unsigned int size,
			int channelCount = 3,
			StorageType storageType = StorageType::Float16,
			bool mipmap = true);

		void CreateFromFiles(
			const std::vector<std::string>& files,
			int channelCount = 0,
			StorageType storageType = StorageType::Float16,
			bool mipmap = true);

		void CreateFromFiles(
			const std::string& name,
			const std::string& extension,
			int channelCount = 0,
			StorageType storageType = StorageType::Float16,
			bool mipmap = true);

		void ComputeMipmap();

		//void CreateFomHDR(const Texture& hdrTexture);

		~GlCubemap();

		void Bind(unsigned int slot = 0) const;
		void Unbind() const;

		inline unsigned int GlId() const { return m_gl_id; }
		inline bool IsHDR() const { return m_storageType == StorageType::Float16 || m_storageType == StorageType::Float32; }
		inline int GetSize() const { return m_size; }
	};
}