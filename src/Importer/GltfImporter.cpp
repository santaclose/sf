#include "GltfImporter.h"

#include <fstream>
#include <iostream>
#include <tiny_gltf.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sf::GltfImporter
{
	std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> nodeToBonePerModel;
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
	DataType positionDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Position)->dataType;
	DataType normalDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Normal)->dataType;
	DataType uvsDataType = mesh.vertexLayout.GetComponent(VertexAttribute::TexCoords)->dataType;

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

			size_t primitiveVertexCount = 0;
			// Vertices
			{
				const float* positionBuffer = nullptr;
				const float* normalsBuffer = nullptr;
				const float* texCoordsBuffer = nullptr;
				const float* boneWeightsBuffer = nullptr;
				const void* jointsBuffer = nullptr;

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
				if (prim.attributes.find("WEIGHTS_0") != prim.attributes.end())
				{
					const tinygltf::Accessor& accessor = model.accessors[prim.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
					boneWeightsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				int jointComponentType;
				if (prim.attributes.find("JOINTS_0") != prim.attributes.end())
				{
					const tinygltf::Accessor& accessor = model.accessors[prim.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
					jointComponentType = accessor.componentType;
					jointsBuffer = &(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]);
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
					glm::vec3* posPtr = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, vertexStart + i);
					*posPtr = { positionBuffer[i * 3 + 0], positionBuffer[i * 3 + 1], positionBuffer[i * 3 + 2] };
					if (normalsBuffer)
					{
						glm::vec3* normalPtr = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Normal, vertexStart + i);
						*normalPtr = glm::normalize(glm::vec3(normalsBuffer[i * 3 + 0], normalsBuffer[i * 3 + 1], normalsBuffer[i * 3 + 2]));
					}
					if (texCoordsBuffer)
					{
						glm::vec2* uvsPtr = (glm::vec2*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::TexCoords, vertexStart + i);
						*uvsPtr = { texCoordsBuffer[i * 2 + 0], 1.0 - texCoordsBuffer[i * 2 + 1] };
					}
					if (jointsBuffer && mesh.vertexLayout.GetComponent(VertexAttribute::BoneIndices) != nullptr)
					{
						DataType boneIndicesDataType = mesh.vertexLayout.GetComponent(VertexAttribute::BoneIndices)->dataType;
						assert(nodeToBonePerModel.find(id) != nodeToBonePerModel.end()); // need mapping from gltf node to bone index to set vertex bone indices
						if (jointComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
						{
							const uint8_t* buf = static_cast<const uint8_t*>(jointsBuffer);
							glm::vec4* boneIndicesPtr = (glm::vec4*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::BoneIndices, vertexStart + i);
							*boneIndicesPtr = {
								(float)nodeToBonePerModel[id][model.skins[0].joints[buf[i * 4 + 0]]],
								(float)nodeToBonePerModel[id][model.skins[0].joints[buf[i * 4 + 1]]],
								(float)nodeToBonePerModel[id][model.skins[0].joints[buf[i * 4 + 2]]],
								(float)nodeToBonePerModel[id][model.skins[0].joints[buf[i * 4 + 3]]] };
						}
						else // unsigned short
						{
							const uint16_t* buf = static_cast<const uint16_t*>(jointsBuffer);
							glm::vec4* boneIndicesPtr = (glm::vec4*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::BoneIndices, vertexStart + i);
							*boneIndicesPtr = {
								(float)nodeToBonePerModel[id][model.skins[0].joints[buf[i * 4 + 0]]],
								(float)nodeToBonePerModel[id][model.skins[0].joints[buf[i * 4 + 1]]],
								(float)nodeToBonePerModel[id][model.skins[0].joints[buf[i * 4 + 2]]],
								(float)nodeToBonePerModel[id][model.skins[0].joints[buf[i * 4 + 3]]] };
						}
					}
					if (boneWeightsBuffer && mesh.vertexLayout.GetComponent(VertexAttribute::BoneWeights) != nullptr)
					{
						glm::vec4* boneWeightsPtr = (glm::vec4*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::BoneWeights, vertexStart + i);
						*boneWeightsPtr = { boneWeightsBuffer[i * 4 + 0], boneWeightsBuffer[i * 4 + 1], boneWeightsBuffer[i * 4 + 2], boneWeightsBuffer[i * 4 + 3] };
					}
				}
			}
			// Indices
			{
				if (prim.indices < 0)
				{
					int startIndex = mesh.indexVector.size();
					mesh.pieces.push_back(startIndex);
					mesh.indexVector.resize(startIndex + primitiveVertexCount);
					for (int i = startIndex; i < mesh.indexVector.size(); i++)
						mesh.indexVector[i] = vertexStart + (i - startIndex);
				}
				else
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
	void GenerateSkeletonProcessNodes(const tinygltf::Model& model, int node, SkeletonData& skeleton, std::unordered_map<uint32_t, uint32_t>& nodeToBone, int parentBone = -1)
	{
		if (node < 0)
		{
			for (int node = 0; node < model.nodes.size(); node++)
				GenerateSkeletonProcessNodes(model, node, skeleton, nodeToBone, parentBone);
			return;
		}

		skeleton.bones.emplace_back();
		Bone& currentBone = skeleton.bones.back();

		if (model.nodes[node].scale.size() == 3)
			currentBone.scale = glm::max(glm::max(model.nodes[node].scale[0], model.nodes[node].scale[1]), model.nodes[node].scale[2]);
		if (model.nodes[node].translation.size() == 3)
			currentBone.translation = glm::make_vec3(model.nodes[node].translation.data());
		if (model.nodes[node].rotation.size() == 4)
			currentBone.rotation = glm::make_quat(model.nodes[node].rotation.data());
		if (model.nodes[node].matrix.size() == 16)
			currentBone.localMatrix = glm::make_mat4x4(model.nodes[node].matrix.data());
		else
			currentBone.localMatrix = glm::translate(glm::mat4(1.0f), currentBone.translation) * glm::mat4(currentBone.rotation) * glm::scale(glm::mat4(1.0f), glm::vec3(currentBone.scale));

		int currentBoneIndex = skeleton.bones.size() - 1;
		nodeToBone[node] = currentBoneIndex;
		currentBone.parent = parentBone;

		for (int child : model.nodes[node].children)
			GenerateSkeletonProcessNodes(model, child, skeleton, nodeToBone, currentBoneIndex);
	}
}

void sf::GltfImporter::GenerateSkeleton(int id, SkeletonData& skeleton, int index)
{
	skeleton.bones.clear();

	tinygltf::Model& model = *(models[id]);

	// generate skeleton
	int rootBoneNode = model.skins[index].skeleton;
	nodeToBonePerModel[id] = std::unordered_map<uint32_t, uint32_t>();
	GenerateSkeletonProcessNodes(model, rootBoneNode, skeleton, nodeToBonePerModel[id]);

	// copy inverse model matrix for each bone
	std::vector<glm::mat4> invModelMatricesTemp;
	const tinygltf::Accessor& accessor = model.accessors[model.skins[index].inverseBindMatrices];
	const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
	const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
	invModelMatricesTemp.resize(accessor.count);
	memcpy(invModelMatricesTemp.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
	for (size_t i = 0; i < model.skins[index].joints.size(); i++)
		skeleton.bones[nodeToBonePerModel[id][model.skins[index].joints[i]]].invModelMatrix = invModelMatricesTemp[i];

	std::cout << "[GltfImporter] Generated skeleton with " << skeleton.bones.size() << " bones\n";

	// reserve space for skinning matrices
	skeleton.skinningMatrices.resize(skeleton.bones.size());

	// load animations
	for (tinygltf::Animation& anim : model.animations)
	{
		skeleton.animations.emplace_back();
		SkeletalAnimation& skeletonAnimation = skeleton.animations.back();
		for (auto& samp : anim.samplers)
		{
			skeletonAnimation.samplers.emplace_back();
			AnimationSampler& skeletonAnimationSampler = skeletonAnimation.samplers.back();

			if (samp.interpolation == "LINEAR")
				skeletonAnimationSampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
			if (samp.interpolation == "STEP")
				skeletonAnimationSampler.interpolation = AnimationSampler::InterpolationType::STEP;
			if (samp.interpolation == "CUBICSPLINE")
				skeletonAnimationSampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;

			// Read sampler input time values
			{
				const tinygltf::Accessor& accessor = model.accessors[samp.input];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				const float* buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++)
					skeletonAnimationSampler.inputs.push_back(buf[index]);

				for (auto input : skeletonAnimationSampler.inputs)
				{
					if (input < skeletonAnimation.start)
						skeletonAnimation.start = input;
					if (input > skeletonAnimation.end)
						skeletonAnimation.end = input;
				}
			}

			// Read sampler output T/R/S values 
			{
				const tinygltf::Accessor& accessor = model.accessors[samp.output];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

				switch (accessor.type)
				{
				case TINYGLTF_TYPE_VEC3:
				{
					const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
						skeletonAnimationSampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
					break;
				}
				case TINYGLTF_TYPE_VEC4:
				{
					const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
						skeletonAnimationSampler.outputsVec4.push_back(buf[index]);
					break;
				}
				default:
				{
					std::cout << "[GltfImporter] Unknown type for animation sampler output" << std::endl;
					break;
				}
				}
			}
		}

		// Channels
		for (auto& source : anim.channels)
		{
			skeletonAnimation.channels.emplace_back();
			AnimationChannel& skeletonAnimationChannel = skeletonAnimation.channels.back();

			if (source.target_path == "rotation")
				skeletonAnimationChannel.path = AnimationChannel::PathType::ROTATION;
			if (source.target_path == "translation")
				skeletonAnimationChannel.path = AnimationChannel::PathType::TRANSLATION;
			if (source.target_path == "scale")
				skeletonAnimationChannel.path = AnimationChannel::PathType::SCALE;
			if (source.target_path == "weights")
			{
				std::cout << "[GltfImporter] Animated weights not supported, skipping channel" << std::endl;
				continue;
			}
			skeletonAnimationChannel.samplerIndex = source.sampler;
			if (nodeToBonePerModel[id].find(source.target_node) != nodeToBonePerModel[id].end())
				skeletonAnimationChannel.bone = nodeToBonePerModel[id][source.target_node];
			else
				std::cout << "[GltfImporter] Target node not found\n";
		}
		std::cout << "[GltfImporter] Imported " << skeletonAnimation.samplers.size() << " samplers and " << skeletonAnimation.channels.size() << " channels\n";
	}

	std::cout << "[GltfImporter] Imported " << skeleton.animations.size() << " animations\n";
}