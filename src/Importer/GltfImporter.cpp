#include "GltfImporter.h"

#include <fstream>
#include <iostream>
#include <tiny_gltf.h>
#include <glad/glad.h>

#include <MeshProcessor.h>

namespace sf::GltfImporter {

	std::vector<tinygltf::Model*> models;
}

int sf::GltfImporter::Load(const std::string& filePath)
{
	tinygltf::Model* newModel = new tinygltf::Model();

	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret;
	std::string ext = filePath.substr(filePath.find_last_of(".") + 1);

	bool isBinary = ext == "glb";
	if (ext != "gltf" && !isBinary)
	{
		std::cout << "[GltfImporter] Invalid file\n";
		return -1;
	}

	if (isBinary)
		ret = loader.LoadBinaryFromFile(newModel, &err, &warn, filePath);
	else
		ret = loader.LoadASCIIFromFile(newModel, &err, &warn, filePath);

	if (!warn.empty())
		printf("[GltfImporter] Warn: %s\n", warn.c_str());

	if (!err.empty())
		printf("[GltfImporter] Err: %s\n", err.c_str());

	if (!ret)
		printf("[GltfImporter] Failed to parse glTF\n");


	models.push_back(newModel);
	return models.size() - 1;
}

void sf::GltfImporter::Destroy(int id)
{
	delete models[id];
	models[id] = nullptr;
}

// https://github.com/SaschaWillems/Vulkan/blob/master/examples/gltfloading/gltfloading.cpp
void sf::GltfImporter::GenerateMeshData(int id, MeshData& meshData)
{
	assert(id > -1 && id < models.size());
	assert(models[id] != nullptr);

	tinygltf::Model& model = *(models[id]);

	std::vector<glm::vec3> vertex_positions;
	std::vector<glm::vec3> vertex_normals;
	std::vector<glm::vec2> vertex_uv;

	for (const tinygltf::Mesh& gltfMesh : model.meshes)
	{
		for (auto& prim : gltfMesh.primitives)
		{
			uint32_t vertexStart = static_cast<uint32_t>(meshData.vertexVector.size());
			uint32_t indexCount = 0;

			// Vertices
			{
				const float* positionBuffer = nullptr;
				const float* normalsBuffer = nullptr;
				const float* texCoordsBuffer = nullptr;
				size_t vertexCount = 0;

				if (prim.attributes.find("POSITION") != prim.attributes.end())
				{
					const tinygltf::Accessor& accessor = model.accessors[prim.attributes.find("POSITION")->second];
					const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
					positionBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					vertexCount = accessor.count;
				}
				if (prim.attributes.find("NORMAL") != prim.attributes.end())
				{
					const tinygltf::Accessor& accessor = model.accessors[prim.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
					normalsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				if (prim.attributes.find("TEXCOORD_0") != prim.attributes.end())
				{
					const tinygltf::Accessor& accessor = model.accessors[prim.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
					texCoordsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}

				// Create Vertices
				for (int i = 0; i < vertexCount; i++)
				{
					Vertex v;
					v.position = { positionBuffer[i * 3 + 0], positionBuffer[i * 3 + 1], positionBuffer[i * 3 + 2] };
					if (normalsBuffer)
						v.normal = glm::normalize(glm::vec3(normalsBuffer[i * 3 + 0], normalsBuffer[i * 3 + 1], normalsBuffer[i * 3 + 2]));
					if (texCoordsBuffer)
						v.textureCoord = { texCoordsBuffer[i * 2 + 0], 1.0 - texCoordsBuffer[i * 2 + 1] };
					meshData.vertexVector.push_back(v);
				}
			}
			// Indices
			{
				const tinygltf::Accessor& accessor = model.accessors[prim.indices];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				indexCount += static_cast<uint32_t>(accessor.count);

				// glTF supports different component types of indices
				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
					uint32_t* buf = new uint32_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
					meshData.pieces.push_back(meshData.indexVector.size());
					for (size_t index = 0; index < accessor.count; index++)
						meshData.indexVector.push_back(buf[index] + vertexStart);
					delete[] buf;
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					uint16_t* buf = new uint16_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
					meshData.pieces.push_back(meshData.indexVector.size());
					for (size_t index = 0; index < accessor.count; index++)
						meshData.indexVector.push_back(buf[index] + vertexStart);
					delete[] buf;
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					uint8_t* buf = new uint8_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
					meshData.pieces.push_back(meshData.indexVector.size());
					for (size_t index = 0; index < accessor.count; index++)
						meshData.indexVector.push_back(buf[index] + vertexStart);
					delete[] buf;
					break;
				}
				default:
					std::cerr << "[GltfImporter] Index component type " << accessor.componentType << " not supported!" << std::endl;
					return;
				}
			}
		}
	}
}

void sf::GltfImporter::GenerateTexture(int id, int textureIndex, Texture& texture)
{
	assert(id > -1 && id < models.size());
	assert(models[id] != nullptr);
	assert(textureIndex > -1 && textureIndex < models[id]->textures.size());

	glGenTextures(1, &texture.gl_id);

	tinygltf::Image& image = models[id]->images[models[id]->textures[textureIndex].source];

	glBindTexture(GL_TEXTURE_2D, texture.gl_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLenum format = GL_RGBA;

	texture.channelCount = image.component;

	if (image.component == 1) {
		format = GL_RED;
	}
	else if (image.component == 2) {
		format = GL_RG;
	}
	else if (image.component == 3) {
		format = GL_RGB;
	}
	else {
		// ???
	}

	GLenum type = GL_UNSIGNED_BYTE;
	if (image.bits == 8) {
		texture.storageType = Texture::StorageType::UnsignedByte;
		// ok
	}
	else if (image.bits == 16) {
		type = GL_UNSIGNED_SHORT;
	}
	else {
		// ???
	}
	texture.width = image.width;
	texture.height = image.height;
	texture.standardImgBuffer = &image.image.at(0);
	texture.needToFreeBuffer = false;
	texture.wrapMode = Texture::WrapMode::Repeat;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format, type, &image.image.at(0));

	// mipmapping
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}