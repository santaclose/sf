#include "Material.h"

#include <fstream>
#include <iostream>

#include <DataTypes.h>
#include <Bitmap.h>

namespace sf {

	std::unordered_map<std::string, uint32_t> typeStringToType = {
		{"b", (uint32_t)DataType::b},
		{"i8", (uint32_t)DataType::i8},
		{"i16", (uint32_t)DataType::i16},
		{"i32", (uint32_t)DataType::i32},
		{"i64", (uint32_t)DataType::i64},
		{"u8", (uint32_t)DataType::u8},
		{"u16", (uint32_t)DataType::u16},
		{"u32", (uint32_t)DataType::u32},
		{"u64", (uint32_t)DataType::u64},
		{"f16", (uint32_t)DataType::f16},
		{"f32", (uint32_t)DataType::f32},
		{"f64", (uint32_t)DataType::f64},
		{"vec2f32", (uint32_t)DataType::vec2f32},
		{"vec3f32", (uint32_t)DataType::vec3f32},
		{"vec4f32", (uint32_t)DataType::vec4f32},
		{"mat2f32", (uint32_t)DataType::mat2f32},
		{"mat3f32", (uint32_t)DataType::mat3f32},
		{"mat4f32", (uint32_t)DataType::mat4f32},
		{"vec2f64", (uint32_t)DataType::vec2f64},
		{"vec3f64", (uint32_t)DataType::vec3f64},
		{"vec4f64", (uint32_t)DataType::vec4f64},
		{"mat2f64", (uint32_t)DataType::mat2f64},
		{"mat3f64", (uint32_t)DataType::mat3f64},
		{"mat4f64", (uint32_t)DataType::mat4f64},
		{"bitmap", (uint32_t)ShaderDataType::bitmap},
		{"cubemap", (uint32_t)ShaderDataType::cubemap}
	};
}

sf::Material::Material(bool isDoubleSided)
{
	this->isDoubleSided = isDoubleSided;
}

sf::Material::Material(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath, bool isDoubleSided)
{
	this->isDoubleSided = isDoubleSided;
	this->vertexShaderFilePath = vertexShaderFilePath;
	this->fragmentShaderFilePath = fragmentShaderFilePath;
}

sf::Material::Material(const std::string& filePath, bool isDoubleSided)
{
	this->isDoubleSided = isDoubleSided;

	std::ifstream ifs(filePath);

	if (ifs.fail())
		std::cout << "[Material] Could not read material file: " << filePath << std::endl;

	std::string fileContents((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	int i = 0;
	while (i < fileContents.length())
	{
		if (fileContents.substr(i, 8) == "shader: ")
		{
			int q, p;
			q = p = i + 9;
			while (fileContents[p] != '\"') { p++; }
			vertexShaderFilePath = fileContents.substr(q, p - q);
			q = p + 1;
			while (fileContents[q] != '\"') { q++; }
			q++;
			p = q;
			while (fileContents[p] != '\"') { p++; }
			fragmentShaderFilePath = fileContents.substr(q, p - q);
		}
		else if (
			fileContents[i] >= 'A' && fileContents[i] <= 'Z' ||
			fileContents[i] >= 'a' && fileContents[i] <= 'z') // check if line has something
		{
			int q, p;
			q = p = i;
			while (fileContents[p] != ' ') { p++; }
			uint32_t uniformDataType = typeStringToType[fileContents.substr(q, p - q)];
			q = p = p + 1;
			while (fileContents[p] != ':') { p++; }
			std::string uniformName = fileContents.substr(q, p - q);

			if (fileContents[p + 2] == '_') // is renderer uniform which means renderer is responsible for setting it
			{
				q = p + 2;
				while (p < fileContents.length() && fileContents[p] != '\n') p++;
				std::string rendererUniformValueString = fileContents.substr(q, p - q);
				if (rendererUniformValueString.compare("_IRRADIANCE_MAP_") == 0)
					rendererUniforms[uniformName] = { uniformDataType, RendererUniformData::IrradianceMap };
				else if (rendererUniformValueString.compare("_PREFILTER_MAP_") == 0)
					rendererUniforms[uniformName] = { uniformDataType, RendererUniformData::PrefilterMap };
				else if (rendererUniformValueString.compare("_BRDF_LUT_") == 0)
					rendererUniforms[uniformName] = { uniformDataType, RendererUniformData::BrdfLUT };
			}
			else
			{
				switch (uniformDataType)
				{
				case (uint32_t)DataType::b:
					if (fileContents[p + 2] == 't')
						uniforms[uniformName] = { uniformDataType, (void*)true };
					else
						uniforms[uniformName] = { uniformDataType, (void*)false };
					break;
				case (uint32_t)ShaderDataType::bitmap:
					p += 3;
					q = p;
					while (fileContents[p] != '\"') { p++; }
					std::string imageFilePath = fileContents.substr(q, p - q);
					if (p + 1 < fileContents.length() && fileContents[p + 1] >= '0' && fileContents[p + 1] <= '9')
					{
						int channelToUse = fileContents[p + 1] - '0';
						Bitmap tempBitmap(imageFilePath);

						Bitmap* newBitmap = new Bitmap(tempBitmap.dataType, 1, tempBitmap.width, tempBitmap.height);
						allocatedMemory.insert(newBitmap);
						uniforms[uniformName] = { uniformDataType, newBitmap };
						((Bitmap*)uniforms[uniformName].data)->CopyChannel(tempBitmap, channelToUse, 0);
					}
					else
					{
						Bitmap* newBitmap = new Bitmap(imageFilePath);
						allocatedMemory.insert(newBitmap);
						uniforms[uniformName] = { uniformDataType, newBitmap };
					}
					break;
				}
			}
		}

		while (true) // go to next line
		{
			i++;
			if (i >= fileContents.length())
				break;
			if (fileContents[i - 1] == '\n')
				break;
		}
	}
}

sf::Material::~Material()
{
	for (void* p : allocatedMemory)
		delete p;
}