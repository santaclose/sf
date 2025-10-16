#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

#include <BufferLayout.h>

namespace sf
{
	struct Uniform
	{
		DataType dataType;
		union {
			void* p;
			float f32;
			int32_t i32;
			uint32_t u32;
		} data;
	};

	enum class RendererUniformData
	{
		IrradianceMap = 0,
		PrefilterMap = 1,
		BrdfLUT = 2
	};
	struct RendererUniform
	{
		DataType dataType;
		RendererUniformData data;
	};
	enum class MaterialBlendMode
	{
		Alpha, Multiply, Add
	};
	enum class MaterialDrawMode
	{
		Fill, Lines, Points
	};

	struct Material
	{
		std::string vertShaderFilePath, fragShaderFilePath, tescShaderFilePath, teseShaderFilePath;
		std::unordered_map<std::string, Uniform> uniforms;
		std::unordered_map<std::string, RendererUniform> rendererUniforms;
		bool isDoubleSided = false;
		MaterialDrawMode drawMode = MaterialDrawMode::Fill;
		MaterialBlendMode blendMode = MaterialBlendMode::Alpha;
		BufferLayout* particleBufferLayout = nullptr;
		BufferLayout* voxelBufferLayout = nullptr;
		uint32_t tessPatchVertexCount;
		std::string tessSpacing;
		std::string tessWinding;
	private:
		std::unordered_set<void*> allocatedBitmaps;
	public:
		Material() = default;
		void CreateFromFile(const std::string& filePath);
		inline bool UsesTessellation() const { return tescShaderFilePath.length() > 0; }
		~Material();
	};
}