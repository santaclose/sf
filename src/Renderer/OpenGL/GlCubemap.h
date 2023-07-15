#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace sf {

	class GlCubemap
	{
	public:
		enum StorageType {
			UnsignedByte, Float16, Float32
		};

	public:
		bool isInitialized = false;
		uint32_t gl_id;
		int size;
		StorageType storageType;

		void GetGlEnums(int channelCount, StorageType storageType, unsigned int& type, int& internalFormat, unsigned int& format);

	public:
		void Create(
			uint32_t size,
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

		~GlCubemap();

		void Bind(uint32_t slot = 0) const;
		void Unbind() const;

		inline bool IsHDR() const { return storageType == StorageType::Float16 || storageType == StorageType::Float32; }
	};
}