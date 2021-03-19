#include "ModelLoader.h"
#include <fstream>
#include <iostream>
#include <tiny_gltf.h>

void ComputeTangentSpace(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector)
{
	for (int i = 0; i < indexVector.size(); i+=3)
	{
		// tangent space
		glm::vec3 edge1 = vertexVector[indexVector[i + 1]].position - vertexVector[indexVector[i]].position;
		glm::vec3 edge2 = vertexVector[indexVector[i + 2]].position - vertexVector[indexVector[i]].position;
		glm::vec2 deltaUV1 = vertexVector[indexVector[i + 1]].textureCoord - vertexVector[indexVector[i]].textureCoord;
		glm::vec2 deltaUV2 = vertexVector[indexVector[i + 2]].textureCoord - vertexVector[indexVector[i]].textureCoord;

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

		vertexVector[indexVector[i]].tangent += t;
		vertexVector[indexVector[i + 1]].tangent += t;
		vertexVector[indexVector[i + 2]].tangent += t;

		vertexVector[indexVector[i]].bitangent += b;
		vertexVector[indexVector[i + 1]].bitangent += b;
		vertexVector[indexVector[i + 2]].bitangent += b;
	}
}

void ModelLoader::LoadGltfFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor, bool isBinary)
{
	using namespace tinygltf;

	tinygltf::Model model;
	TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret;
	if (isBinary)
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, filePath);
	else
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);

	if (!warn.empty())
		printf("[tinygltf] Warn: %s\n", warn.c_str());

	if (!err.empty())
		printf("[tinygltf] Err: %s\n", err.c_str());

	if (!ret)
		printf("[tinygltf] Failed to parse glTF\n");

	std::vector<glm::vec3> vertex_positions;
	std::vector<glm::vec3> vertex_normals;
	std::vector<glm::vec2> vertex_uv;

	for (auto& x : model.meshes)
	{
		for (auto& prim : x.primitives)
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
						vertex_positions[i] *= scaleFactor;
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
						vertex_uv[i].y = 1.0-tex.y;
					}
				}
			}
		}
	}

	vertexVector.resize(vertex_positions.size());
	for (int i = 0; i < vertex_positions.size(); i++)
	{
		//std::cout << vertex_uv[i].x << ", " << vertex_uv[i].y << ", " << vertex_uv[i].z << std::endl;
		vertexVector[i].position = vertex_positions[i];
		vertexVector[i].normal = vertex_normals[i];
		vertexVector[i].textureCoord = vertex_uv[i];
	}
	ComputeTangentSpace(vertexVector, indexVector);
}