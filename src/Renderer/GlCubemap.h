#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <DataTypes.h>

namespace sf {

	class GlCubemap
	{
	public:
		bool isInitialized = false;
		uint32_t gl_id;
		int size;
		DataType storageDataType;

		void Create(
			uint32_t size,
			int channelCount = 3,
			DataType storageDataType = DataType::f16,
			bool mipmap = true);

		void CreateFromFiles(
			const std::vector<std::string>& files,
			int channelCount = 0,
			DataType storageDataType = DataType::f16,
			bool mipmap = true);

		void CreateFromFiles(
			const std::string& name,
			const std::string& extension,
			int channelCount = 0,
			DataType storageDataType = DataType::f16,
			bool mipmap = true);

		void Delete();
		GlCubemap() = default;
		~GlCubemap() = default;

		void ComputeMipmap();

		void Bind(uint32_t slot = 0) const;
		void Unbind() const;

		inline bool IsHDR() const { return storageDataType == DataType::f16 || storageDataType == DataType::f32; }
	};
}