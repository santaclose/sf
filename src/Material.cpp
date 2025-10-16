#include "Material.h"

#include <fstream>
#include <iostream>

#include <DataTypes.h>
#include <Bitmap.h>

namespace sf {

	std::unordered_map<std::string, DataType> typeStringToType = {
		{"b", DataType::b},
		{"i8", DataType::i8},
		{"i16", DataType::i16},
		{"i32", DataType::i32},
		{"i64", DataType::i64},
		{"u8", DataType::u8},
		{"u16", DataType::u16},
		{"u32", DataType::u32},
		{"u64", DataType::u64},
		{"f16", DataType::f16},
		{"f32", DataType::f32},
		{"f64", DataType::f64},
		{"vec2f32", DataType::vec2f32},
		{"vec3f32", DataType::vec3f32},
		{"vec4f32", DataType::vec4f32},
		{"mat2f32", DataType::mat2f32},
		{"mat3f32", DataType::mat3f32},
		{"mat4f32", DataType::mat4f32},
		{"vec2f64", DataType::vec2f64},
		{"vec3f64", DataType::vec3f64},
		{"vec4f64", DataType::vec4f64},
		{"mat2f64", DataType::mat2f64},
		{"mat3f64", DataType::mat3f64},
		{"mat4f64", DataType::mat4f64},
		{"bitmap", DataType::bitmap},
		{"cubemap", DataType::cubemap}
	};
}

void sf::Material::CreateFromFile(const std::string& filePath)
{
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
			int spaces[3] = { -1, -1, -1 };
			int p;
			int currentSpace = 0;
			for (p = i + 9; fileContents[p] != '\n'; p++)
				if (fileContents[p] == ' ')
					spaces[currentSpace++] = p;

			if (spaces[2] == -1)
			{
				vertShaderFilePath = fileContents.substr(i + 9, spaces[0] - 1 - i - 9);
				fragShaderFilePath = fileContents.substr(spaces[0] + 2, p - spaces[0] - 1 - 2);
			}
			else
			{
				vertShaderFilePath = fileContents.substr(i + 9, spaces[0] - 1 - i - 9);
				tescShaderFilePath = fileContents.substr(spaces[0] + 2, spaces[1] - spaces[0] - 1 - 2);
				teseShaderFilePath = fileContents.substr(spaces[1] + 2, spaces[2] - spaces[1] - 1 - 2);
				fragShaderFilePath = fileContents.substr(spaces[2] + 2, p - spaces[2] - 1 - 2);
			}
		}
		else if (fileContents.substr(i, 14) == "tessellation: ")
		{
			int q, p;
			q = p = i + 14;
			for (; fileContents[p] != ' '; p++);
			tessPatchVertexCount = std::stoi(fileContents.substr(q, p - q));
			for (p = q = p + 1; fileContents[p] != ' '; p++);
			tessSpacing = fileContents.substr(q, p - q);
			for (p = q = p + 1; fileContents[p] != '\n'; p++);
			tessWinding = fileContents.substr(q, p - q);
		}
		else if (
			fileContents[i] >= 'A' && fileContents[i] <= 'Z' ||
			fileContents[i] >= 'a' && fileContents[i] <= 'z') // check if line has something
		{
			int q, p;
			q = p = i;
			while (fileContents[p] != ' ') { p++; }
			DataType uniformDataType = typeStringToType[fileContents.substr(q, p - q)];
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
				uniforms[uniformName].dataType = uniformDataType;
				switch (uniformDataType)
				{
				case DataType::b:
					uniforms[uniformName].data.u32 = fileContents[p + 2] == 't';
					break;
				case DataType::f32:
					for (q = p = p + 2; fileContents[p] != '\n'; p++);
					uniforms[uniformName].data.f32 = std::stof(fileContents.substr(q, p - q));
					break;
				case DataType::bitmap:
					for (q = p = p + 3; fileContents[p] != '\"'; p++);
					std::string imageFilePath = fileContents.substr(q, p - q);
					if (p + 1 < fileContents.length() && fileContents[p + 1] >= '0' && fileContents[p + 1] <= '9')
					{
						int channelToUse = fileContents[p + 1] - '0';
						Bitmap tempBitmap;
						tempBitmap.CreateFromFile(imageFilePath);

						Bitmap* newBitmap = new Bitmap();
						newBitmap->CreateSolid(tempBitmap.dataType, 1, tempBitmap.width, tempBitmap.height);
						allocatedBitmaps.insert(newBitmap);
						newBitmap->CopyChannel(tempBitmap, channelToUse, 0);
						uniforms[uniformName].data.p = newBitmap;
					}
					else
					{
						Bitmap* newBitmap = new Bitmap();
						newBitmap->CreateFromFile(imageFilePath);
						allocatedBitmaps.insert(newBitmap);
						uniforms[uniformName].data.p = newBitmap;
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
			if (fileContents[i - 1] == '\n' && fileContents[i] != '#')
				break;
		}
	}
}

sf::Material::~Material()
{
	for (void* p : allocatedBitmaps)
		delete (Bitmap*) p;
}