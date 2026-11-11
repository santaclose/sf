#include "MeshProcessor.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <set>

#include <Random.h>
#include <Geometry/Geometry.h>

#include <meshoptimizer.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

namespace sf {

	template<typename PDT, typename NDT>
	void ComputeNormalsT(MeshData& mesh)
	{
		// set all normals to zero
		for (uint32_t i = 0; i < mesh.vertexCount; i++)
		{
			NDT* targetPointer = mesh.AccessVertexComponent<NDT>(Buffer::Component::Normal, i);
			targetPointer->x = targetPointer->y = targetPointer->z = 0.0;
		}

		// compute normals from face vertex positions
		for (int i = 0; i < mesh.indexCount; i += 3)
		{
			NDT faceNormal;
			const PDT* a = mesh.ReadVertexComponent<PDT>(Buffer::Component::Position, mesh.indexBuffer[i + 0]);
			const PDT* b = mesh.ReadVertexComponent<PDT>(Buffer::Component::Position, mesh.indexBuffer[i + 1]);
			const PDT* c = mesh.ReadVertexComponent<PDT>(Buffer::Component::Position, mesh.indexBuffer[i + 2]);
			NDT ab = (*b) - (*a);
			NDT ac = (*c) - (*a);
			faceNormal = glm::normalize(glm::cross(ab, ac));
			NDT* normalA = mesh.AccessVertexComponent<NDT>(Buffer::Component::Normal, mesh.indexBuffer[i + 0]);
			NDT* normalB = mesh.AccessVertexComponent<NDT>(Buffer::Component::Normal, mesh.indexBuffer[i + 1]);
			NDT* normalC = mesh.AccessVertexComponent<NDT>(Buffer::Component::Normal, mesh.indexBuffer[i + 2]);
			*normalA += faceNormal;
			*normalB += faceNormal;
			*normalC += faceNormal;
		}

		/* Normalize */
		for (uint32_t i = 0; i < mesh.vertexCount; i++)
		{
			NDT* targetPointer = mesh.AccessVertexComponent<NDT>(Buffer::Component::Normal, i);
			*targetPointer = glm::normalize(*targetPointer);
		}
	}

	template<typename PDT, typename TDT, typename UDT>
	void ComputeTangentSpaceT(MeshData& mesh)
	{
		// set all tangents to zero
		for (uint32_t i = 0; i < mesh.vertexCount; i++)
		{
			TDT* targetPointer;
			targetPointer = mesh.AccessVertexComponent<TDT>(Buffer::Component::Tangent, i);
			targetPointer->x = targetPointer->y = targetPointer->z = 0.0;
		}

		for (uint32_t i = 0; i < mesh.indexCount; i += 3)
		{
			const PDT* ap = mesh.ReadVertexComponent<PDT>(Buffer::Component::Position, mesh.indexBuffer[i + 0]);
			const PDT* bp = mesh.ReadVertexComponent<PDT>(Buffer::Component::Position, mesh.indexBuffer[i + 1]);
			const PDT* cp = mesh.ReadVertexComponent<PDT>(Buffer::Component::Position, mesh.indexBuffer[i + 2]);
			const UDT* au = mesh.ReadVertexComponent<UDT>(Buffer::Component::UV, mesh.indexBuffer[i + 0]);
			const UDT* bu = mesh.ReadVertexComponent<UDT>(Buffer::Component::UV, mesh.indexBuffer[i + 1]);
			const UDT* cu = mesh.ReadVertexComponent<UDT>(Buffer::Component::UV, mesh.indexBuffer[i + 2]);
			TDT* at = mesh.AccessVertexComponent<TDT>(Buffer::Component::Tangent, mesh.indexBuffer[i + 0]);
			TDT* bt = mesh.AccessVertexComponent<TDT>(Buffer::Component::Tangent, mesh.indexBuffer[i + 1]);
			TDT* ct = mesh.AccessVertexComponent<TDT>(Buffer::Component::Tangent, mesh.indexBuffer[i + 2]);

			glm::dvec3 edge1 = *bp - *ap;
			glm::dvec3 edge2 = *cp - *ap;
			glm::dvec2 deltaUV1 = *bu - *au;
			glm::dvec2 deltaUV2 = *cu - *au;

			double f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			glm::dvec3 t, b;

			t.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			t.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			t.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			t = glm::normalize(t);

			b.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			b.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			b.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
			b = glm::normalize(b);

			*at += t;
			*bt += t;
			*ct += t;
		}
	}
}

void sf::MeshProcessor::ComputeNormals(MeshData& mesh)
{
	DataType positionDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position)->dataType;
	DataType normalDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Normal)->dataType;

	assert(positionDataType == DataType::vec3f32 || positionDataType == DataType::vec3f64);
	assert(normalDataType == DataType::vec3f32 || normalDataType == DataType::vec3f64);

	if (positionDataType == DataType::vec3f32)
	{
		if (normalDataType == DataType::vec3f32)
			ComputeNormalsT<glm::vec3, glm::vec3>(mesh);
		else
			ComputeNormalsT<glm::vec3, glm::dvec3>(mesh);
	}
	else
	{
		if (normalDataType == DataType::vec3f32)
			ComputeNormalsT<glm::dvec3, glm::vec3>(mesh);
		else
			ComputeNormalsT<glm::dvec3, glm::dvec3>(mesh);
	}
}

void sf::MeshProcessor::ComputeTangentSpace(MeshData& mesh)
{
	DataType positionDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position)->dataType;
	DataType uvsDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::UV)->dataType;
	DataType tangentDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Tangent)->dataType;

	assert(positionDataType == DataType::vec3f32 || positionDataType == DataType::vec3f64);
	assert(uvsDataType == DataType::vec2f32 || uvsDataType == DataType::vec2f64);
	assert(tangentDataType == DataType::vec3f32 || tangentDataType == DataType::vec3f64);

	// support doubles as well
	if (positionDataType == DataType::vec3f32)
	{
		if (tangentDataType == DataType::vec3f32)
		{
			if (uvsDataType == DataType::vec2f32)
				ComputeTangentSpaceT<glm::vec3, glm::vec3, glm::vec2>(mesh);
			else
				ComputeTangentSpaceT<glm::vec3, glm::vec3, glm::dvec2>(mesh);
		}
		else
		{
			if (uvsDataType == DataType::vec2f32)
				ComputeTangentSpaceT<glm::vec3, glm::dvec3, glm::vec2>(mesh);
			else
				ComputeTangentSpaceT<glm::vec3, glm::dvec3, glm::dvec2>(mesh);
		}
	}
	else
	{
		if (tangentDataType == DataType::vec3f32)
		{
			if (uvsDataType == DataType::vec2f32)
				ComputeTangentSpaceT<glm::dvec3, glm::vec3, glm::vec2>(mesh);
			else
				ComputeTangentSpaceT<glm::dvec3, glm::vec3, glm::dvec2>(mesh);
		}
		else
		{
			if (uvsDataType == DataType::vec2f32)
				ComputeTangentSpaceT<glm::dvec3, glm::dvec3, glm::vec2>(mesh);
			else
				ComputeTangentSpaceT<glm::dvec3, glm::dvec3, glm::dvec2>(mesh);
		}
	}
}

float sf::MeshProcessor::ComputeOcclusion(const std::vector<std::pair<bool, float>>& rayResults, float maxDistance, float falloff)
{
	float brightness = 1.0f;
	int hit_count = 0;
	for (const auto& r : rayResults)
	{
		if (r.first) // did hit
		{
			float normalizedDistance = r.second / maxDistance;
			float occlusion = 1.0f - std::pow(normalizedDistance, falloff);
			brightness -= occlusion / (float)rayResults.size();
		}
	}

	brightness = glm::min(1.0f, brightness * glm::sqrt(2.0f));
	return brightness;
}

void sf::MeshProcessor::ComputeVertexAmbientOcclusion(MeshData& mesh, const VoxelVolumeData* voxelVolume, const VertexAmbientOcclusionBakerConfig* config)
{
	DataType positionDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position)->dataType;
	DataType aoDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::AO)->dataType;

	assert(positionDataType == DataType::vec3f32);
	assert(aoDataType == DataType::f32);

	VertexAmbientOcclusionBakerConfig autoConfig;
	if (config == nullptr)
	{
		if (voxelVolume != nullptr)
		{
			glm::vec3 volumeSize = glm::vec3(voxelVolume->voxelCountPerAxis) * voxelVolume->voxelSize;
			autoConfig.rayDistance = volumeSize.length();
		}
		else
		{
			glm::vec3 minvpos, maxvpos;
			minvpos = maxvpos = *mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, 0);
			for (int i = 1; i < mesh.vertexCount; i++)
			{
				glm::vec3 vertexPos = *mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, i);
				minvpos.x = std::min(vertexPos.x, minvpos.x);
				minvpos.y = std::min(vertexPos.y, minvpos.y);
				minvpos.z = std::min(vertexPos.z, minvpos.z);
				maxvpos.x = std::max(vertexPos.x, maxvpos.x);
				maxvpos.y = std::max(vertexPos.y, maxvpos.y);
				maxvpos.z = std::max(vertexPos.z, maxvpos.z);
			}
			glm::vec3 cornertocorner = maxvpos - minvpos;
			autoConfig.rayDistance = cornertocorner.length();
		}
		autoConfig.falloff = autoConfig.rayDistance * 1.1f;
		config = &autoConfig;
	}

	#pragma omp parallel for
	for (int q = 0; q < mesh.vertexCount; q++)
	{
		glm::vec3 vertexPos = *mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, q);
		float* aoTarget = mesh.AccessVertexComponent<float>(Buffer::Component::AO, q);

		std::vector<std::pair<bool, float>> rayResults(config->rayCount);
		for (int i = 0; i < config->rayCount; i++)
		{
			glm::vec3 rayDir = Random::UnitVec3();
			if (config->onlyCastRaysUpwards && rayDir.y < 0.0f)
				rayDir.y = -rayDir.y;

			bool didHit = false;
			float distance;
			if (voxelVolume != nullptr)
			{
				didHit = voxelVolume->CastRay(vertexPos + (rayDir * config->rayOriginOffset), rayDir, true, &distance) != nullptr;
				if (distance > config->rayDistance)
					distance = config->rayDistance;
				rayResults[i] = { didHit, distance };
			}
			else
			{
				for (int j = 0; j < mesh.indexCount && !didHit; j += 3) // for each face, intersect
				{
					if (mesh.indexBuffer[j + 0] == q || mesh.indexBuffer[j + 1] == q || mesh.indexBuffer[j + 2] == q)
						continue; // current vertex belongs to this face

					Geometry::RayHit rh;
					didHit = Geometry::IntersectRayTriangle(vertexPos + (rayDir * config->rayOriginOffset), rayDir,
						*mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, mesh.indexBuffer[j + 0]),
						*mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, mesh.indexBuffer[j + 1]),
						*mesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, mesh.indexBuffer[j + 2]),
						&rh);
					if (didHit)
						distance = rh.distance;
				}
				if (distance > config->rayDistance)
					distance = config->rayDistance;
			}
		}
		*aoTarget = ComputeOcclusion(rayResults, config->rayDistance, config->falloff);
	}

	for (int pass = 0; pass < config->denoisePasses; pass++)
	{
		for (int i = 0; i < mesh.indexCount; i += 3)
		{
			float average =
				(*mesh.ReadVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 0]) +
					*mesh.ReadVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 1]) +
					*mesh.ReadVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 2])) / 3.0f;

			*mesh.AccessVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 0]) = glm::mix(*mesh.ReadVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 0]), average, config->denoiseWeight);
			*mesh.AccessVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 1]) = glm::mix(*mesh.ReadVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 1]), average, config->denoiseWeight);
			*mesh.AccessVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 2]) = glm::mix(*mesh.ReadVertexComponent<float>(Buffer::Component::AO, mesh.indexBuffer[i + 2]), average, config->denoiseWeight);
		}
	}
}

void sf::MeshProcessor::AmbientOcclusionBitmapFromVoxelVolume(const VoxelVolumeData& voxelVolume, Bitmap& bitmap, const VertexAmbientOcclusionBakerConfig* config)
{
	glm::uvec3 res = voxelVolume.voxelCountPerAxis;
	bitmap.Destroy();
	bitmap.CreateSolid(DataType::u8, 1, res.x, res.y, res.z);
	assert(bitmap.width == voxelVolume.voxelCountPerAxis.x);
	assert(bitmap.height == voxelVolume.voxelCountPerAxis.y);
	assert(bitmap.depth == voxelVolume.voxelCountPerAxis.z);

	VertexAmbientOcclusionBakerConfig autoConfig;
	if (config == nullptr)
	{
		glm::vec3 volumeSize = glm::vec3(voxelVolume.voxelCountPerAxis) * voxelVolume.voxelSize;
		autoConfig.rayDistance = volumeSize.length();
		autoConfig.falloff = autoConfig.rayDistance * 1.1f;
		autoConfig.denoisePasses = 30;
		autoConfig.onlyCastRaysUpwards = true;
		config = &autoConfig;
	}

	#pragma omp parallel for
	for (int i = 0; i < bitmap.width; i++)
		for (int j = 0; j < bitmap.height; j++)
			for (int k = 0; k < bitmap.depth; k++)
			{
				glm::vec3 centerOfVoxel = voxelVolume.offset + glm::vec3(
					voxelVolume.voxelSize * (0.5f + (float) i),
					voxelVolume.voxelSize * (0.5f + (float) j),
					voxelVolume.voxelSize * (0.5f + (float) k));

				std::vector<std::pair<bool, float>> rayResults(config->rayCount);
				for (int i = 0; i < config->rayCount; i++)
				{
					glm::vec3 rayDir = Random::UnitVec3();
					if (config->onlyCastRaysUpwards && rayDir.y < 0.0f)
						rayDir.y = -rayDir.y;

					bool didHit = false;
					float distance;
					didHit = voxelVolume.CastRay(centerOfVoxel + (rayDir * config->rayOriginOffset), rayDir, false, &distance) != nullptr;
					if (distance > config->rayDistance)
						distance = config->rayDistance;
					rayResults[i] = { didHit, distance };
				}
				float ao = ComputeOcclusion(rayResults, config->rayDistance, config->falloff);
				uint8_t aou8 = (uint8_t) (ao * 255.0f);
				bitmap.SetPixel({i, j, k}, &aou8);
			}

	for (int pass = 0; pass < config->denoisePasses; pass++)
	{
		for (int i = 0; i < bitmap.width; i++)
			for (int j = 0; j < bitmap.height; j++)
				for (int k = 0; k < bitmap.depth; k++)
				{
					float sum = (float) *bitmap.AccessPixel({i, j, k});
					uint32_t count = 1;
					if (i > 0 && (voxelVolume.GetVoxel({i - 1, j, k}) != nullptr) == (voxelVolume.GetVoxel({i, j, k}) != nullptr))
					{
						sum += (float) *bitmap.AccessPixel({i - 1, j, k});
						count++;
					}
					if (i < bitmap.width - 1 && (voxelVolume.GetVoxel({i + 1, j, k}) != nullptr) == (voxelVolume.GetVoxel({i, j, k}) != nullptr))
					{
						sum += (float) *bitmap.AccessPixel({i + 1, j, k});
						count++;
					}
					if (j > 0 && (voxelVolume.GetVoxel({i, j - 1, k}) != nullptr) == (voxelVolume.GetVoxel({i, j, k}) != nullptr))
					{
						sum += (float) *bitmap.AccessPixel({i, j - 1, k});
						count++;
					}
					if (j < bitmap.height - 1 && (voxelVolume.GetVoxel({i, j + 1, k}) != nullptr) == (voxelVolume.GetVoxel({i, j, k}) != nullptr))
					{
						sum += (float) *bitmap.AccessPixel({i, j + 1, k});
						count++;
					}
					if (k > 0 && (voxelVolume.GetVoxel({i, j, k - 1}) != nullptr) == (voxelVolume.GetVoxel({i, j, k}) != nullptr))
					{
						sum += (float) *bitmap.AccessPixel({i, j, k - 1});
						count++;
					}
					if (k < bitmap.depth - 1 && (voxelVolume.GetVoxel({i, j, k + 1}) != nullptr) == (voxelVolume.GetVoxel({i, j, k}) != nullptr))
					{
						sum += (float) *bitmap.AccessPixel({i, j, k + 1});
						count++;
					}
					float average = sum / (float) count;
					*bitmap.AccessPixel({i, j, k}) = (uint8_t) glm::mix((float) *bitmap.AccessPixel({i, j, k}), average, config->denoiseWeight);
				}
	}
}

void sf::MeshProcessor::GenerateGrid(Buffer::Layout vertexLayout, MeshData& meshData, uint32_t sizeX, uint32_t sizeY, uint32_t texResX, uint32_t texResY, float cellSize, bool useQuads)
{
	assert(sizeX > 1u && sizeY > 1u);
	float uvAdjustX = 1.0f / texResX / 2.0f;
	float uvAdjustY = 1.0f / texResY / 2.0f;
	DataType positionDataType = vertexLayout.GetComponentInfo(Buffer::Component::Position)->dataType;
	DataType uvDataType = vertexLayout.GetComponentInfo(Buffer::Component::UV)->dataType;
	assert(positionDataType == DataType::vec3f32);
	assert(uvDataType == DataType::vec2f32);
	uint32_t indicesPerCell = useQuads ? 4u : 6u;

	uint32_t indexCount = (sizeX - 1u) * (sizeY - 1u) * indicesPerCell;
	std::vector<uint32_t> indices(indexCount);
	uint32_t vertexCount = sizeX * sizeY;
	uint8_t* vertexBuffer = new uint8_t[vertexCount * vertexLayout.GetSize()];
	uint32_t vertexCountPerPrimitive = useQuads ? 4u : 3u;

	uint32_t currentIndex = 0u;
	for (int32_t y = 0; y < sizeY; y++)
	{
		for (int32_t x = 0; x < sizeX; x++)
		{
			glm::vec3* currentVertexPos = vertexLayout.Access<glm::vec3>(vertexBuffer, Buffer::Component::Position, y * sizeX + x);
			glm::vec2* currentVertexUV = vertexLayout.Access<glm::vec2>(vertexBuffer, Buffer::Component::UV, y * sizeX + x);
			currentVertexPos->x = (float) x * cellSize;
			currentVertexPos->z = (float) (-y) * cellSize;
			currentVertexPos->y = 0.0f;
			currentVertexUV->x = (float)x / (float)(sizeX - 1) * ((float)(texResX - 1) / (float)texResX) + uvAdjustX;
			currentVertexUV->y = (float)y / (float)(sizeY - 1) * ((float)(texResY - 1) / (float)texResY) + uvAdjustY;
			if (x > 0u && y > 0u)
			{
				assert(useQuads && currentIndex + 3u < indexCount || !useQuads && currentIndex + 5u < indexCount);
				indices[currentIndex++] = (y - 1u) * sizeX + (x - 1u);
				indices[currentIndex++] = (y - 1u) * sizeX + (x - 0u);
				indices[currentIndex++] = (y - 0u) * sizeX + (x - 0u);
				if (!useQuads)
				{
					indices[currentIndex++] = (y - 1u) * sizeX + (x - 1u);
					indices[currentIndex++] = (y - 0u) * sizeX + (x - 0u);
				}
				indices[currentIndex++] = (y - 0u) * sizeX + (x - 1u);
			}
		}
	}

	meshData.Destroy();
	meshData.Create(vertexLayout, vertexBuffer, vertexCount, indices, nullptr, vertexCountPerPrimitive);
}

void sf::MeshProcessor::RemoveUnusedBones(MeshData& mesh, SkeletonData& skeleton)
{
	// Mesh must have bone indices
	assert(mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::BoneIndices) != nullptr);

	// Find used bones, both in mesh and animations
	std::unordered_set<uint32_t> bonesUsedByMesh;
	std::unordered_set<uint32_t> bonesUsedByAnimation;
	for (uint32_t i = 0; i < mesh.vertexCount; i++)
	{
		const glm::vec4* boneIndicesPointer = mesh.ReadVertexComponent<glm::vec4>(Buffer::Component::BoneIndices, i);
		bonesUsedByMesh.insert((uint32_t)boneIndicesPointer->x);
		bonesUsedByMesh.insert((uint32_t)boneIndicesPointer->y);
		bonesUsedByMesh.insert((uint32_t)boneIndicesPointer->z);
		bonesUsedByMesh.insert((uint32_t)boneIndicesPointer->w);
	}
	for (const Animation::SkeletalAnimation& sa : skeleton.m_animations)
	{
		for (const Animation::Channel& ch : sa.channels)
			bonesUsedByAnimation.insert(ch.bone);
	}
	std::unordered_set<uint32_t> usedBones = bonesUsedByMesh;
	usedBones.insert(bonesUsedByAnimation.begin(), bonesUsedByAnimation.end());

	// Find unused but required bones
	std::unordered_map<uint32_t, uint32_t> depthPerBone;
	std::unordered_map<uint32_t, uint32_t> boneRemapping;
	std::unordered_set<uint32_t> requiredUnusedParents;
	for (uint32_t i = 0; i < skeleton.GetBoneCount(); i++)
	{
		uint32_t currentParent = skeleton.GetBoneData(i).parent;
		if (currentParent != -1 &&
			usedBones.find(i) != usedBones.end() &&
			usedBones.find(currentParent) == usedBones.end())
		{
			requiredUnusedParents.insert(currentParent);
			while (true)
			{
				currentParent = skeleton.GetBoneData(currentParent).parent;
				if (currentParent == -1)
					break;
				requiredUnusedParents.insert(currentParent);
			}
		}
	}

	// Compute bone remapping
	uint32_t newBoneIndex = 0;
#ifdef SF_DEBUG
	printf("[MeshProcessor] Remove unused bones draft:\n");
#endif
	for (uint32_t i = 0; i < skeleton.GetBoneCount(); i++)
	{
#ifdef SF_DEBUG
		uint32_t currentParent = skeleton.GetBoneData(i).parent;
		if (currentParent == -1)
			depthPerBone[i] = 0;
		else
			depthPerBone[i] = depthPerBone[currentParent] + 1;
		for (uint32_t j = 0; j < depthPerBone[i] + 1; j++)
			printf("   ");
#endif
		if (usedBones.find(i) != usedBones.end())
		{
#ifdef SF_DEBUG
			bool usedByMesh = bonesUsedByMesh.find(i) != bonesUsedByMesh.end();
			bool usedByAnimation = bonesUsedByAnimation.find(i) != bonesUsedByAnimation.end();
			printf("used bone (%u -> %u) (mesh: %u, anim: %u)\n", i, newBoneIndex, usedByMesh, usedByAnimation);
#endif
			boneRemapping[i] = newBoneIndex;
			newBoneIndex++;
		}
		else
		{
			if (requiredUnusedParents.find(i) != requiredUnusedParents.end())
			{
#ifdef SF_DEBUG
				printf("unused, but REQUIRED bone (%u -> %u)\n", i, newBoneIndex);
#endif
				boneRemapping[i] = newBoneIndex;
				newBoneIndex++;
			}
#ifdef SF_DEBUG
			else
				printf("UNUSED bone (%u)\n", i);
#endif
		}
	}
	std::unordered_set<uint32_t> requiredBones = usedBones;
	requiredBones.insert(requiredUnusedParents.begin(), requiredUnusedParents.end());

	uint32_t requiredBoneCount = (uint32_t)requiredBones.size();
	uint32_t totalBoneCount = (uint32_t)skeleton.GetBoneCount();
	assert(totalBoneCount >= requiredBoneCount);
	if (totalBoneCount == requiredBoneCount)
	{
		printf("[MeshProcessor] All %u bones are in use, skipping\n", totalBoneCount);
		return;
	}
	printf("[MeshProcessor] %u bones are required, and there are %u bones in total, removing %u\n", requiredBoneCount, totalBoneCount, totalBoneCount - requiredBoneCount);

	// Update skeleton
	std::vector<Transform> newBoneLocalTransforms(newBoneIndex);
	std::vector<Transform> newBoneTransforms(newBoneIndex);
	std::vector<BoneData> newBoneData(newBoneIndex);
	for (uint32_t i = 0; i < skeleton.GetBoneCount(); i++)
	{
		if (requiredBones.find(i) == requiredBones.end())
			continue;

		uint32_t targetBone = boneRemapping[i];
		newBoneLocalTransforms[targetBone] = skeleton.GetBoneLocalTransform(i);
		newBoneTransforms[targetBone] = skeleton.GetBoneTransform(i);
		newBoneData[targetBone] = skeleton.GetBoneData(i);
		if (skeleton.GetBoneData(i).parent != -1)
			newBoneData[targetBone].parent = boneRemapping[skeleton.GetBoneData(i).parent];
	}
	skeleton.m_core.m_skinningMatrices.resize(newBoneIndex);
	for (Animation::SkeletalAnimation& sa : skeleton.m_animations)
	{
		for (Animation::Channel& ch : sa.channels)
			ch.bone = boneRemapping[ch.bone];
	}
	for (Animation::Node& an : skeleton.m_nodes)
		an.single.boneCount = newBoneIndex;

	std::unordered_map<uint32_t, TwoBoneIkData> newIkData;
	for (const auto& [shoulderBone, data] : skeleton.m_ikData)
	{
		newIkData[boneRemapping[shoulderBone]] = data;
		newIkData[boneRemapping[shoulderBone]].elbowBone = boneRemapping[newIkData[boneRemapping[shoulderBone]].elbowBone];
	}
	skeleton.m_core.m_boneLocalTransforms = newBoneLocalTransforms;
	skeleton.m_core.m_boneTransforms = newBoneTransforms;
	skeleton.m_core.m_boneData = newBoneData;
	skeleton.m_ikData = newIkData;

	// Update mesh
	for (uint32_t i = 0; i < mesh.vertexCount; i++)
	{
		glm::vec4* boneIndicesPointer = mesh.AccessVertexComponent<glm::vec4>(Buffer::Component::BoneIndices, i);
		boneIndicesPointer->x = (float) boneRemapping[(uint32_t)boneIndicesPointer->x];
		boneIndicesPointer->y = (float) boneRemapping[(uint32_t)boneIndicesPointer->y];
		boneIndicesPointer->z = (float) boneRemapping[(uint32_t)boneIndicesPointer->z];
		boneIndicesPointer->w = (float) boneRemapping[(uint32_t)boneIndicesPointer->w];
	}
}

void sf::MeshProcessor::Decimate(const MeshData& inputMesh, MeshData& outputMesh, float ratio, float error)
{
	assert(&inputMesh != &outputMesh);
	// Vertices must have positions
	assert(inputMesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position) != nullptr);
	assert(inputMesh.vertexCountPerPrimitive == 3);
	assert(ratio >= 0.0f && ratio <= 1.0f);

	uint32_t pieceCount = inputMesh.pieceCount;
	outputMesh.Destroy();
	outputMesh = inputMesh;
	outputMesh.usesExternalVertexBuffer = true;

	uint32_t totalIndexCount = 0;
	std::vector<std::pair<uint32_t, uint32_t*>> perPiece(pieceCount);
	const glm::vec3* firstVertexPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, 0);
	for (uint32_t i = 0; i < pieceCount; i++)
	{
		uint32_t pieceStart, pieceEnd;
		inputMesh.GetPieceLimits(i, pieceStart, pieceEnd);
		uint32_t pieceIndexCount = pieceEnd - pieceStart;
		uint32_t* newIndices = new uint32_t[pieceIndexCount];
		uint32_t targetIndexCount = (uint32_t(pieceIndexCount * ratio) / 3) * 3;
		uint32_t actualIndexCount = meshopt_simplify(
			newIndices,
			inputMesh.indexBuffer + pieceStart,
			pieceIndexCount,
			&(firstVertexPos->x),
			inputMesh.vertexCount,
			inputMesh.vertexBufferLayout.GetSize(),
			targetIndexCount,
			error
		);
		uint32_t* tightIndices = new uint32_t[actualIndexCount];
		memcpy(tightIndices, newIndices, sizeof(uint32_t) * actualIndexCount);
		delete[] newIndices;
		perPiece[i] = { actualIndexCount, tightIndices };
		totalIndexCount += actualIndexCount;
	}

	uint32_t pieceIndex = 0;
	uint32_t currentOffset = 0;
	uint32_t* finalIndexBuffer = new uint32_t[totalIndexCount];
	uint32_t* pieceBuffer = new uint32_t[perPiece.size()];
	for (auto& pieceInfo : perPiece)
	{
		pieceBuffer[pieceIndex] = currentOffset;
		memcpy(finalIndexBuffer + currentOffset, pieceInfo.second, sizeof(uint32_t) * pieceInfo.first);
		currentOffset += pieceInfo.first;
		delete[] pieceInfo.second;
		pieceIndex++;
	}

	outputMesh.indexCount = totalIndexCount;
	outputMesh.indexBuffer = finalIndexBuffer;
	outputMesh.pieces = pieceBuffer;
}

void sf::MeshProcessor::Remesh(const MeshData& inputMesh, MeshData& outputMesh, int resolution)
{
	static Buffer::Layout positionOnlyVertexLayout = Buffer::Layout({
		Buffer::Component::Position
	});

	const glm::vec3* firstVertexPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, 0);
	const unsigned int options = meshopt_RemeshSolve;

	uint32_t totalIndexCount = 0;
	uint32_t totalVertexCount = 0;
	std::vector<std::vector<uint32_t>> indices(inputMesh.pieceCount);
	std::vector<std::vector<glm::vec3>> positions(inputMesh.pieceCount);
	for (uint32_t i = 0; i < inputMesh.pieceCount; i++)
	{
		uint32_t pieceStart, pieceEnd;
		inputMesh.GetPieceLimits(i, pieceStart, pieceEnd);
		uint32_t pieceIndexCount = pieceEnd - pieceStart;

		size_t capacity = meshopt_remesh(nullptr, 0, inputMesh.indexBuffer + pieceStart, pieceIndexCount, &firstVertexPos->x, inputMesh.vertexCount, inputMesh.vertexBufferLayout.GetSize(), resolution, options);
		assert(capacity != 0);
		std::vector<float> triangles(capacity * 3 * 3);
		size_t triCount = meshopt_remesh(&triangles[0], capacity, inputMesh.indexBuffer + pieceStart, pieceIndexCount, &firstVertexPos->x, inputMesh.vertexCount, inputMesh.vertexBufferLayout.GetSize(), resolution, options);

		/* Build indexed mesh */
		std::unordered_map<glm::vec3, uint32_t> posIndexMap;
		for (uint32_t j = 0; j < triCount * 3; j++)
		{
			glm::vec3 currentVertexPos = glm::vec3(triangles[j * 3 + 0], triangles[j * 3 + 1], triangles[j * 3 + 2]);
			if (posIndexMap.find(currentVertexPos) != posIndexMap.end())
			{
				indices[i].push_back(posIndexMap[currentVertexPos]);
			}
			else
			{
				uint32_t newIndex = positions.back().size() + totalVertexCount;
				indices[i].push_back(newIndex);
				positions[i].push_back(currentVertexPos);
				posIndexMap[positions[i].back()] = newIndex;
			}
		}

		totalVertexCount += positions[i].size();
		totalIndexCount += indices[i].size();
	}

	uint32_t vertexOffset = 0;
	uint32_t indexOffset = 0;
	uint8_t* vertexBuffer = new uint8_t[totalVertexCount * sizeof(glm::vec3)];
	std::vector<uint32_t> finalIndices(totalIndexCount);
	std::vector<uint32_t> finalPieces;
	for (uint32_t i = 0; i < inputMesh.pieceCount; i++)
	{
		finalPieces.push_back(indexOffset);
		memcpy(vertexBuffer + vertexOffset * sizeof(glm::vec3), positions[i].data(), positions[i].size() * sizeof(glm::vec3));
		memcpy(finalIndices.data() + indexOffset, indices[i].data(), indices[i].size() * sizeof(uint32_t));
		vertexOffset += positions[i].size();
		indexOffset += indices[i].size();
	}

	outputMesh.Destroy();
	outputMesh.Create(positionOnlyVertexLayout, vertexBuffer, totalVertexCount, finalIndices, &finalPieces);
}

void sf::MeshProcessor::TransformMesh(MeshData& mesh, const Transform& transform)
{
	DataType positionDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position)->dataType;
	DataType normalDataType;
	DataType tangentDataType;
	bool hasNormal = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Normal) != nullptr;
	bool hasTangent = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Tangent) != nullptr;
	if (hasNormal)
		normalDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Normal)->dataType;
	if (hasTangent)
		tangentDataType = mesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Tangent)->dataType;

	assert(positionDataType == DataType::vec3f32);
	assert(!hasNormal || normalDataType == DataType::vec3f32);
	assert(!hasTangent || tangentDataType == DataType::vec3f32);

	for (uint32_t i = 0; i < mesh.vertexCount; i++)
	{
		{
			glm::vec3* targetPointer = mesh.AccessVertexComponent<glm::vec3>(Buffer::Component::Position, i);
			*targetPointer = transform.ApplyToPoint(*targetPointer);
		}

		if (hasNormal)
		{
			glm::vec3* targetPointer = mesh.AccessVertexComponent<glm::vec3>(Buffer::Component::Normal, i);
			*targetPointer = transform.ApplyToDirection(*targetPointer);
		}

		if (hasTangent)
		{
			glm::vec3* targetPointer = mesh.AccessVertexComponent<glm::vec3>(Buffer::Component::Tangent, i);
			*targetPointer = transform.ApplyToDirection(*targetPointer);
		}
	}
	mesh.ComputeBoundingGeometry();
}

void sf::MeshProcessor::TransformSkeleton(SkeletonData& skeleton, const Transform& transform)
{
	glm::mat4 transformMatrix = transform.ComputeMatrix();

	Transform& blt = skeleton.m_core.m_boneLocalTransforms[0];
	Transform newBlt = transform;
	newBlt.Apply(blt);
	blt = newBlt;

	for (BoneData& bd : skeleton.m_core.m_boneData)
		bd.invModelTransform.Apply(transform.Inverse());

	for (Animation::SkeletalAnimation& a : skeleton.m_animations)
	{
		for (Animation::Channel& c : a.channels)
		{
			/* We only need to transform the root */
			if (skeleton.GetBoneData(c.bone).parent >= 0)
			    continue;

			switch (c.path)
			{
			case Animation::Channel::PathType::TRANSLATION:
				for (glm::vec4& output : a.samplers[c.samplerIndex].outputsVec4)
				{
					glm::vec3 newTranslation = transform.ApplyToPoint(glm::vec3(output));
					output = glm::vec4(newTranslation.x, newTranslation.y, newTranslation.z, 1.0f);
				}
				break;
			case Animation::Channel::PathType::ROTATION:
				for (glm::vec4& output : a.samplers[c.samplerIndex].outputsVec4)
				{
					glm::quat newRotation = transform.rotation * glm::make_quat(glm::value_ptr(output));
					output = glm::vec4(newRotation.x, newRotation.y, newRotation.z, newRotation.w);
				}
				break;
			case Animation::Channel::PathType::SCALE:
				for (glm::vec4& output : a.samplers[c.samplerIndex].outputsVec4)
				{
					float originalScale = glm::max(glm::max(output.x, output.y), output.z);
					float newScale = transform.scale * originalScale;
					output = glm::vec4(newScale, newScale, newScale, newScale);
				}
				break;
			}
		}
	}
}

void sf::MeshProcessor::TransferVertexAttributesFromClosest(const MeshData& inputMesh, MeshData& outputMesh)
{
	assert(inputMesh.vertexBufferLayout == outputMesh.vertexBufferLayout);

	const Buffer::LayoutInfo& layout = inputMesh.vertexBufferLayout.GetInfo();
	for (int i = 0; i < outputMesh.vertexCount; i++)
	{
		const glm::vec3* outPos = outputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, i);
		float dis2 = std::numeric_limits<float>::max();
		int wantedIndex = 0;
		for (int j = 0; j < inputMesh.vertexCount; j++)
		{
			const glm::vec3* inPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, j);
			float currentDis2 = glm::distance2(*inPos, *outPos);
			if (currentDis2 < dis2)
			{
				dis2 = currentDis2;
				wantedIndex = j;
			}
		}

		for (const auto& [component, componentInfo] : layout.components)
		{
			if (component == Buffer::Component::Position)
				continue;
			switch (componentInfo.dataType)
			{
			case DataType::vec4f32:
				*outputMesh.AccessVertexComponent<glm::vec4>(component, i) = *inputMesh.ReadVertexComponent<glm::vec4>(component, wantedIndex);
				break;
			case DataType::vec3f32:
				*outputMesh.AccessVertexComponent<glm::vec3>(component, i) = *inputMesh.ReadVertexComponent<glm::vec3>(component, wantedIndex);
				break;
			case DataType::vec2f32:
				*outputMesh.AccessVertexComponent<glm::vec2>(component, i) = *inputMesh.ReadVertexComponent<glm::vec2>(component, wantedIndex);
				break;
			case DataType::f32:
				*outputMesh.AccessVertexComponent<float>(component, i) = *inputMesh.ReadVertexComponent<float>(component, wantedIndex);
				break;
			default:
				printf("[MeshProcessor] Skipping transfer of vertex attribute: %u\n", (uint32_t) component);
				break;
			}
		}
	}
}

void sf::MeshProcessor::TransferVertexAttributesInterpolatingBlendMatrix(const MeshData& inputMesh, MeshData& outputMesh, uint32_t maxVertices)
{
	assert(inputMesh.vertexBufferLayout == outputMesh.vertexBufferLayout);

	const Buffer::LayoutInfo& layout = inputMesh.vertexBufferLayout.GetInfo();
	uint32_t vertexStride = layout.GetSize();

	#pragma omp parallel for
	for (int i = 0; i < outputMesh.vertexCount; i++)
	{
		const glm::vec3* outPos = outputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, i);

		std::vector<glm::vec3> closestPoints;
		std::vector<uint32_t> closestPointIndices;
		Math::ClosestPoints(*outPos, inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, 0), inputMesh.vertexCount, 20, closestPoints, closestPointIndices, vertexStride);

		assert(closestPoints.size() == 20);
		float* blendMatrix = new float[closestPoints.size() * closestPoints.size() * closestPoints.size()];
		float* weights = new float[closestPoints.size()];
		Math::ComputePointWeightBlendMatrix3D(closestPoints.data(), closestPoints.size(), blendMatrix);
		Math::PointWeightingBlendMatrix3D(*outPos, closestPoints.data(), blendMatrix, weights, closestPoints.size(), vertexStride);
		uint8_t* tempVertices = new uint8_t[vertexStride * closestPoints.size()];
		for (int j = 0; j < closestPoints.size(); j++)
			memcpy(layout.Access(tempVertices, j), inputMesh.ReadVertex(closestPointIndices[j]), vertexStride);

		for (const auto& [component, componentInfo] : layout.components)
		{
			if (component == Buffer::Component::Position)
				continue;
			switch (componentInfo.dataType)
			{
			case DataType::vec3f32:
				Math::WeightedBlend(layout.Access<glm::vec3>(tempVertices, component, 0), weights, closestPoints.size(), *outputMesh.AccessVertexComponent<glm::vec3>(component, i), vertexStride);
				break;
			case DataType::vec2f32:
				Math::WeightedBlend(layout.Access<glm::vec2>(tempVertices, component, 0), weights, closestPoints.size(), *outputMesh.AccessVertexComponent<glm::vec2>(component, i), vertexStride);
				break;
			case DataType::f32:
				Math::WeightedBlend(layout.Access<float>(tempVertices, component, 0), weights, closestPoints.size(), *outputMesh.AccessVertexComponent<float>(component, i), vertexStride);
				break;
			default:
				printf("[MeshProcessor] Skipping transfer of vertex attribute: %u\n", (uint32_t) component);
				break;
			}
		}
		delete[] tempVertices;
		delete[] weights;
		delete[] blendMatrix;
	}

}