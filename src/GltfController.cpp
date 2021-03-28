#include "GltfController.h"
#include <fstream>
#include <iostream>
#include <tiny_gltf.h>
#include <glad/glad.h>

namespace GltfController
{
	std::vector<tinygltf::Model> models;

	void ComputeTangentSpace(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector)
	{
		for (int i = 0; i < indexVector.size(); i += 3)
		{
			// tangent space
			unsigned int ci = indexVector[i + 0];
			unsigned int ni = indexVector[i + 1];
			unsigned int nni = indexVector[i + 2];

			glm::vec3 edge1 = vertexVector[ni].position - vertexVector[ci].position;
			glm::vec3 edge2 = vertexVector[nni].position - vertexVector[ci].position;
			glm::vec2 deltaUV1 = vertexVector[ni].textureCoord - vertexVector[ci].textureCoord;
			glm::vec2 deltaUV2 = vertexVector[nni].textureCoord - vertexVector[ci].textureCoord;

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::vec3 t, b;

			t.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			t.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			t.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			t = glm::normalize(t);

			b.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			b.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			b.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
			b = glm::normalize(b);

			vertexVector[ci].tangent += t;
			vertexVector[ni].tangent += t;
			vertexVector[nni].tangent += t;

			vertexVector[ci].bitangent += b;
			vertexVector[ni].bitangent += b;
			vertexVector[nni].bitangent += b;
		}
	}
}

int GltfController::Load(const std::string& filePath)
{
	models.emplace_back();
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret;
	std::string ext = filePath.substr(filePath.find_last_of(".") + 1);

	bool isBinary = ext == "glb";
	if (ext != "gltf" && !isBinary)
	{
		std::cout << "[GltfController] Invalid file\n";
		return -1;
	}

	if (isBinary)
		ret = loader.LoadBinaryFromFile(&(models[models.size() - 1]), &err, &warn, filePath);
	else
		ret = loader.LoadASCIIFromFile(&(models[models.size() - 1]), &err, &warn, filePath);

	if (!warn.empty())
		printf("[GltfController] Warn: %s\n", warn.c_str());

	if (!err.empty())
		printf("[GltfController] Err: %s\n", err.c_str());

	if (!ret)
		printf("[GltfController] Failed to parse glTF\n");

	return models.size() - 1;
}

void GltfController::Destroy(int id)
{
}

void GltfController::Model(int id, int meshIndex, std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector)
{
	assert(id > -1 && id < models.size());
	assert(meshIndex > -1 && meshIndex < models[id].meshes.size());

	tinygltf::Model& model = models[id];

	std::vector<glm::vec3> vertex_positions;
	std::vector<glm::vec3> vertex_normals;
	std::vector<glm::vec2> vertex_uv;

	for (auto& prim : model.meshes[meshIndex].primitives)
	{
		// Fill indices:
		const tinygltf::Accessor& accessor = model.accessors[prim.indices];
		const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
		int stride = accessor.ByteStride(bufferView);
		size_t indexCount = accessor.count;
		size_t indexOffset = indexVector.size();
		indexVector.resize(indexOffset + indexCount);

		const unsigned char* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;
		int index_remap[3];
		index_remap[0] = 0;
		index_remap[1] = 1;
		index_remap[2] = 2;

		if (stride == 1)
		{
			for (size_t i = 0; i < indexCount; i += 3)
			{
				indexVector[indexOffset + i + 0] = data[i + index_remap[0]];
				indexVector[indexOffset + i + 1] = data[i + index_remap[1]];
				indexVector[indexOffset + i + 2] = data[i + index_remap[2]];
			}
		}
		else if (stride == 2)
		{
			for (size_t i = 0; i < indexCount; i += 3)
			{
				indexVector[indexOffset + i + 0] = ((uint16_t*)data)[i + index_remap[0]];
				indexVector[indexOffset + i + 1] = ((uint16_t*)data)[i + index_remap[1]];
				indexVector[indexOffset + i + 2] = ((uint16_t*)data)[i + index_remap[2]];
			}
		}
		else if (stride == 4)
		{
			for (size_t i = 0; i < indexCount; i += 3)
			{
				indexVector[indexOffset + i + 0] = ((uint32_t*)data)[i + index_remap[0]];
				indexVector[indexOffset + i + 1] = ((uint32_t*)data)[i + index_remap[1]];
				indexVector[indexOffset + i + 2] = ((uint32_t*)data)[i + index_remap[2]];
			}
		}
		else
		{
			assert(0 && "unsupported index stride!");
		}

		for (auto& attr : prim.attributes)
		{
			const std::string& attr_name = attr.first;
			int attr_data = attr.second;

			const tinygltf::Accessor& accessor = model.accessors[attr_data];
			const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

			int stride = accessor.ByteStride(bufferView);
			size_t vertexCount = accessor.count;

			const unsigned char* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

			if (!attr_name.compare("POSITION"))
			{
				vertex_positions.resize(vertexCount);
				assert(stride == 12);
				for (size_t i = 0; i < vertexCount; ++i)
				{
					vertex_positions[i] = ((glm::vec3*)data)[i];
				}
			}
			else if (!attr_name.compare("NORMAL"))
			{
				vertex_normals.resize(vertexCount);
				assert(stride == 12);
				for (size_t i = 0; i < vertexCount; ++i)
				{
					vertex_normals[i] = ((glm::vec3*)data)[i];
				}
			}
			else if (!attr_name.compare("TEXCOORD_0"))
			{
				vertex_uv.resize(vertexCount);
				assert(stride == 8);
				for (size_t i = 0; i < vertexCount; ++i)
				{
					const glm::vec2& tex = ((glm::vec2*)data)[i];

					vertex_uv[i].x = tex.x;
					vertex_uv[i].y = 1.0 - tex.y;
				}
			}
		}
	}

	vertexVector.resize(vertex_positions.size());
	for (int i = 0; i < vertex_positions.size(); i++)
	{
		//std::cout << vertex_uv[i].x << ", " << vertex_uv[i].y << ", " << vertex_uv[i].z << std::endl;
		if (vertex_positions.size() > 0)
			vertexVector[i].position = vertex_positions[i];
		if (vertex_normals.size() > 0)
			vertexVector[i].normal = vertex_normals[i];
		if (vertex_uv.size() > 0)
		{
			vertexVector[i].textureCoord.x = vertex_uv[i].x;
			vertexVector[i].textureCoord.y = vertex_uv[i].y;
		}
	}

	ComputeTangentSpace(vertexVector, indexVector);
}

void GltfController::Texture(int id, int textureIndex, unsigned int& glId, int& width, int& height)
{
	assert(id > -1 && id < models.size());
	assert(textureIndex > -1 && textureIndex < models[id].textures.size());

	glGenTextures(1, &glId);

	tinygltf::Image& image = models[id].images[models[id].textures[textureIndex].source];

	glBindTexture(GL_TEXTURE_2D, glId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLenum format = GL_RGBA;

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
		// ok
	}
	else if (image.bits == 16) {
		type = GL_UNSIGNED_SHORT;
	}
	else {
		// ???
	}
	width = image.width;
	height = image.height;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format, type, &image.image.at(0));

	// mipmapping
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}
