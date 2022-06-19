#pragma once

#include <string>
#include <unordered_set>
#include <unordered_map>

namespace sf
{
	struct Uniform
	{
		uint32_t dataType; // one of those in DataTypes.h
		void* data = nullptr;
	};

	enum class RendererUniformData
	{
		IrradianceMap = 0,
		PrefilterMap = 1,
		BrdfLUT = 2
	};
	struct RendererUniform
	{
		uint32_t dataType;
		RendererUniformData data;
	};

	struct Material
	{
		std::string vertexShaderFilePath, fragmentShaderFilePath;
		std::unordered_map<std::string, Uniform> uniforms;
		std::unordered_map<std::string, RendererUniform> rendererUniforms;
		bool isDoubleSided;
		std::unordered_set<void*> allocatedMemory;

		Material::Material(bool isDoubleSided = false);
		Material::Material(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, bool isDoubleSided = false);
		Material::Material(const std::string& filePath, bool isDoubleSided = false);
		Material::~Material();
	};
}