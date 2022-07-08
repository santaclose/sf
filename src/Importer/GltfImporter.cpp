#include "GltfImporter.h"

#include <fstream>
#include <iostream>
#include <tiny_gltf.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

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
void sf::GltfImporter::GenerateMeshData(int id, MeshData& mesh)
{
	DataType positionDataType = mesh.vertexLayout.GetComponent(MeshData::VertexAttribute::Position)->dataType;
	DataType normalDataType = mesh.vertexLayout.GetComponent(MeshData::VertexAttribute::Normal)->dataType;
	DataType uvsDataType = mesh.vertexLayout.GetComponent(MeshData::VertexAttribute::UV)->dataType;

	assert(positionDataType == DataType::vec3f32);
	assert(normalDataType == DataType::vec3f32);
	assert(uvsDataType == DataType::vec2f32);

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
			uint32_t vertexStart = static_cast<uint32_t>(mesh.vertexCount);
			uint32_t indexCount = 0;

			// Vertices
			{
				const float* positionBuffer = nullptr;
				const float* normalsBuffer = nullptr;
				const float* texCoordsBuffer = nullptr;
				size_t primitiveVertexCount = 0;

				if (prim.attributes.find("POSITION") != prim.attributes.end())
				{
					const tinygltf::Accessor& accessor = model.accessors[prim.attributes.find("POSITION")->second];
					const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
					positionBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					primitiveVertexCount = accessor.count;
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

				void* oldVertexBuffer = mesh.vertexBuffer;
				mesh.vertexCount += primitiveVertexCount;
				mesh.vertexBuffer = malloc(mesh.vertexLayout.GetSize() * mesh.vertexCount);
				if (oldVertexBuffer != nullptr)
					memcpy(mesh.vertexBuffer, oldVertexBuffer, mesh.vertexLayout.GetSize() * vertexStart);
				free(oldVertexBuffer);

				// Create Vertices
				for (uint32_t i = 0; i < primitiveVertexCount; i++)
				{
					glm::vec3* posPtr = (glm::vec3*) mesh.vertexLayout.Access(mesh.vertexBuffer, MeshData::VertexAttribute::Position, vertexStart + i);
					*posPtr = { positionBuffer[i * 3 + 0], positionBuffer[i * 3 + 1], positionBuffer[i * 3 + 2] };
					if (normalsBuffer)
					{
						glm::vec3* normalPtr = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, MeshData::VertexAttribute::Normal, vertexStart + i);
						*normalPtr = glm::normalize(glm::vec3(normalsBuffer[i * 3 + 0], normalsBuffer[i * 3 + 1], normalsBuffer[i * 3 + 2]));
					}
					if (texCoordsBuffer)
					{
						glm::vec2* uvsPtr = (glm::vec2*)mesh.vertexLayout.Access(mesh.vertexBuffer, MeshData::VertexAttribute::UV, vertexStart + i);
						*uvsPtr = { texCoordsBuffer[i * 2 + 0], 1.0 - texCoordsBuffer[i * 2 + 1] };
					}
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
					mesh.pieces.push_back(mesh.indexVector.size());
					for (size_t index = 0; index < accessor.count; index++)
						mesh.indexVector.push_back(buf[index] + vertexStart);
					delete[] buf;
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					uint16_t* buf = new uint16_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
					mesh.pieces.push_back(mesh.indexVector.size());
					for (size_t index = 0; index < accessor.count; index++)
						mesh.indexVector.push_back(buf[index] + vertexStart);
					delete[] buf;
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					uint8_t* buf = new uint8_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
					mesh.pieces.push_back(mesh.indexVector.size());
					for (size_t index = 0; index < accessor.count; index++)
						mesh.indexVector.push_back(buf[index] + vertexStart);
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

void sf::GltfImporter::GenerateBitmap(int id, int index, Bitmap& bitmap)
{
	assert(id > -1 && id < models.size());
	
	tinygltf::Image& image = models[id]->images[models[id]->textures[index].source];

	DataType dataType;
	switch (image.pixel_type)
	{
	case TINYGLTF_COMPONENT_TYPE_BYTE:
		dataType = DataType::i8;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		dataType = DataType::u8;
		break;
	case TINYGLTF_COMPONENT_TYPE_SHORT:
		dataType = DataType::i16;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		dataType = DataType::u16;
		break;
	case TINYGLTF_COMPONENT_TYPE_INT:
		dataType = DataType::i32;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		dataType = DataType::u32;
		break;
	case TINYGLTF_COMPONENT_TYPE_FLOAT:
		dataType = DataType::f32;
		break;
	case TINYGLTF_COMPONENT_TYPE_DOUBLE:
		dataType = DataType::f64;
		break;
	}

	bitmap.dataType = dataType;
	bitmap.channelCount = image.component;
	bitmap.width = image.width;
	bitmap.height = image.height;

	uint32_t dataTypeSize = GetDataTypeSize(dataType);
	free(bitmap.buffer);
	bitmap.buffer = malloc(dataTypeSize * (bitmap.width) * (bitmap.height) * (bitmap.channelCount));
	memcpy(bitmap.buffer, &image.image.at(0), dataTypeSize * bitmap.width * bitmap.height * bitmap.channelCount);
}

namespace sf::GltfImporter
{
	void GenerateSkeletonProcessNodes(const tinygltf::Model& model, int node, SkeletonData& skeleton, int parentNode = -1, int parentBone = -1)
	{
		float scale;
		if (model.nodes[node].scale.size() == 3)
			scale = glm::max(glm::max(model.nodes[node].scale[0], model.nodes[node].scale[1]), model.nodes[node].scale[2]);
		else
			scale = 1.0f;
		glm::vec3 translation = glm::vec3(0.0f);;
		if (model.nodes[node].translation.size() == 3)
			translation = glm::make_vec3(model.nodes[node].translation.data());
		glm::mat4 rotation = glm::mat4(1.0f);
		if (model.nodes[node].rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(model.nodes[node].rotation.data());
			rotation = glm::mat4(q);
		}
		glm::mat4 mat = glm::mat4(1.0f);
		if (model.nodes[node].matrix.size() == 16)
			mat = glm::make_mat4x4(model.nodes[node].matrix.data());

		skeleton.bones.emplace_back();
		int currentBone = skeleton.bones.size() - 1;
		skeleton.bones.back().parent = parentBone;
		skeleton.bones.back().localMatrix = glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), glm::vec3(scale)) * mat;

		for (int child : model.nodes[node].children)
		{
			GenerateSkeletonProcessNodes(model, child, skeleton, node, currentBone);
		}
	}
}

void sf::GltfImporter::GenerateSkeleton(int id, SkeletonData& skeleton, int index)
{
	skeleton.bones.clear();
	
	const tinygltf::Model& model = *(models[id]);

	// find root bones
	std::vector<int> rootBones;
	for (int i = 0; i < model.nodes.size(); i++)
		rootBones.push_back(i);
	for (int i = 0; i < model.nodes.size(); i++)
	{
		for (int child : model.nodes[i].children)
			for (int j = 0; j < rootBones.size(); j++)
				if (rootBones[j] == child)
				{
					rootBones.erase(rootBones.begin() + j);
					break;
				}
	}
	for (int i = 0; i < model.nodes.size(); i++)
	{
		if (model.nodes[i].children.size() == 0)
			for (int j = 0; j < rootBones.size(); j++)
				if (rootBones[j] == i)
				{
					rootBones.erase(rootBones.begin() + j);
					break;
				}
	}

	GenerateSkeletonProcessNodes(model, rootBones[index], skeleton);
}