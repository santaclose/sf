#include "MeshProcessor.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <unordered_set>
#include <unordered_map>

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

void sf::MeshProcessor::Decimate(const MeshData& inputMesh, MeshData& outputMesh, float ratio)
{
	assert(&inputMesh != &outputMesh);
	// Vertices must have positions
	assert(inputMesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position) != nullptr);
	assert(inputMesh.vertexCountPerPrimitive == 3);
	assert(ratio >= 0.0f && ratio <= 1.0f);

	uint32_t targetIndexCount = (uint32_t(inputMesh.indexCount * ratio) / 3) * 3;

	outputMesh.Destroy();
	outputMesh = inputMesh;
	outputMesh.pieces = nullptr;
	outputMesh.pieceCount = 1;
	outputMesh.usesExternalVertexBuffer = true;

	const glm::vec3* firstVertexPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, 0);
	uint32_t* newIndices = new uint32_t[inputMesh.indexCount];
	uint32_t actualIndexCount = meshopt_simplify(
		newIndices,
		inputMesh.indexBuffer,
		inputMesh.indexCount,
		&(firstVertexPos->x),
		inputMesh.vertexCount,
		inputMesh.vertexBufferLayout.GetSize(),
		targetIndexCount,
		1e-2f
	);

	outputMesh.indexCount = actualIndexCount;
	outputMesh.indexBuffer = newIndices;
}

void sf::MeshProcessor::Remesh(const MeshData& inputMesh, MeshData& outputMesh, int resolution)
{
	/* Run mesh optimizer remesh */
	const glm::vec3* firstVertexPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, 0);
	const unsigned int options = meshopt_RemeshSolve;
	size_t capacity = meshopt_remesh(nullptr, 0, inputMesh.indexBuffer, inputMesh.indexCount, &firstVertexPos->x, inputMesh.vertexCount, inputMesh.vertexBufferLayout.GetSize(), resolution, options);
	std::vector<float> triangles(capacity * 3 * 3);
	size_t triCount = meshopt_remesh(&triangles[0], capacity, inputMesh.indexBuffer, inputMesh.indexCount, &firstVertexPos->x, inputMesh.vertexCount, inputMesh.vertexBufferLayout.GetSize(), resolution, options);
	triangles.resize(triCount * 3 * 3);

	/* Build indexed mesh */
	std::vector<uint32_t> indices;
	std::vector<glm::vec3> positions;
	std::unordered_map<glm::vec3, uint32_t> posIndexMap;
	for (uint32_t i = 0; i < triCount * 3; i++)
	{
		glm::vec3 currentVertexPos = glm::vec3(triangles[i * 3 + 0], triangles[i * 3 + 1], triangles[i * 3 + 2]);
		if (posIndexMap.find(currentVertexPos) != posIndexMap.end())
		{
			indices.push_back(posIndexMap[currentVertexPos]);
		}
		else
		{
			indices.push_back(positions.size());
			positions.push_back(currentVertexPos);
			posIndexMap[positions.back()] = indices.back();
		}
	}

	static Buffer::Layout positionOnlyVertexLayout = Buffer::Layout({
		Buffer::Component::Position
	});

	uint8_t* vertexBuffer = new uint8_t[positions.size() * sizeof(glm::vec3)];
	memcpy(vertexBuffer, positions.data(), positions.size() * sizeof(glm::vec3));

	outputMesh.Destroy();
	outputMesh.Create(positionOnlyVertexLayout, vertexBuffer, positions.size(), indices);
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

void sf::MeshProcessor::ApplySkeleton(const glm::mat4* skinMatrixBuffer, const MeshData& inputMesh, MeshData& outputMesh)
{
	assert(inputMesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::BoneIndices) != nullptr);
	assert(inputMesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::BoneWeights) != nullptr);
	assert(outputMesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position) != nullptr);

	for (uint32_t i = 0; i < inputMesh.vertexCount; i++)
	{
		const glm::vec4* indices = inputMesh.ReadVertexComponent<glm::vec4>(Buffer::Component::BoneIndices, i);
		const glm::vec4* weights = inputMesh.ReadVertexComponent<glm::vec4>(Buffer::Component::BoneWeights, i);

		const glm::mat4* matA = &skinMatrixBuffer[(uint32_t) indices->x];
		const glm::mat4* matB = &skinMatrixBuffer[(uint32_t) indices->y];
		const glm::mat4* matC = &skinMatrixBuffer[(uint32_t) indices->z];
		const glm::mat4* matD = &skinMatrixBuffer[(uint32_t) indices->w];

		glm::mat4 skinMatrix = weights->x * *matA + weights->y * *matB + weights->z * *matC + weights->w * *matD;

		const glm::vec3* inPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, i);
		glm::vec3* outPos = outputMesh.AccessVertexComponent<glm::vec3>(Buffer::Component::Position, i);

		*outPos = skinMatrix * glm::vec4(inPos->x, inPos->y, inPos->z, 1.0f);
	}
}


void sf::MeshProcessor::ComputeMeshBoundingSphere(const MeshData& inputMesh, SphereCollider& outputSphere)
{
	glm::vec3 min(std::numeric_limits<float>::max());
	glm::vec3 max(-std::numeric_limits<float>::max());

	for (uint32_t i = 0; i < inputMesh.vertexCount; i++)
	{
		const glm::vec3* vertexPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, i);

		min = glm::min(min, *vertexPos);
		max = glm::max(max, *vertexPos);
	}

	outputSphere.center = (min + max) * 0.5f;

	float maxDistanceSq = 0.0f;
	for (uint32_t i = 0; i < inputMesh.vertexCount; i++)
	{
		const glm::vec3* vertexPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, i);
		float distSq = glm::distance2(outputSphere.center, *vertexPos);
		if (distSq > maxDistanceSq)
			maxDistanceSq = distSq;
	}

	outputSphere.radius = glm::sqrt(maxDistanceSq);
}

void sf::MeshProcessor::ComputeSkinnedMeshBoundingSphere(const SkeletonData& inputSkeleton, const MeshData& inputMesh, SphereCollider& outputSphere)
{
	static const Buffer::Layout positionOnlyVertexLayout = Buffer::Layout({
		Buffer::Component::Position
	});
	MeshData tempM;
	tempM.vertexBufferLayout = positionOnlyVertexLayout;
	tempM.vertexCount = inputMesh.vertexCount;
	tempM.vertexBuffer = new uint8_t[sizeof(glm::vec3) * inputMesh.vertexCount];
	SkeletonCoreData tempS = inputSkeleton.m_core;
	const std::vector<Animation::SkeletalAnimation>& animations = inputSkeleton.m_animations;

	/* Get keyframes */
	std::vector<std::unordered_set<float>> allKeyFrameTimesPerAnimation(animations.size());
	for (uint32_t i = 0; i < animations.size(); i++)
	{
		const Animation::SkeletalAnimation& anim = animations[i];
		for (const Animation::Channel& channel : anim.channels)
		{
			const Animation::Sampler& sampler = anim.samplers[channel.samplerIndex];
			if (sampler.inputs.size() > sampler.outputsVec4.size())
				continue;

			for (float samplerInput : sampler.inputs)
				allKeyFrameTimesPerAnimation[i].insert(samplerInput);
		}
	}

	/* Bounding box containing every animation */
	glm::vec3 min(std::numeric_limits<float>::max());
	glm::vec3 max(-std::numeric_limits<float>::max());
	for (uint32_t i = 0; i < animations.size(); i++)
	{
		const Animation::SkeletalAnimation& anim = animations[i];
		for (float kf : allKeyFrameTimesPerAnimation[i])
		{
			Animation::SampleToBones(anim, tempS.m_boneLocalTransforms.data(), kf);
			tempS.PropagateLocalTransforms(nullptr, true);

			ApplySkeleton(tempS.m_skinningMatrices.data(), inputMesh, tempM);
			for (uint32_t vi = 0; vi < tempM.vertexCount; vi++)
			{
				const glm::vec3* vertexPos = tempM.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, vi);
				min = glm::min(min, *vertexPos);
				max = glm::max(max, *vertexPos);
			}
		}
	}

	/* Bounding sphere containing every animation */
	outputSphere.center = (min + max) * 0.5f;
	float maxDistanceSq = 0.0f;
	for (uint32_t i = 0; i < animations.size(); i++)
	{
		const Animation::SkeletalAnimation& anim = animations[i];
		for (float kf : allKeyFrameTimesPerAnimation[i])
		{
			Animation::SampleToBones(anim, tempS.m_boneLocalTransforms.data(), kf);
			tempS.PropagateLocalTransforms(nullptr, true);

			ApplySkeleton(tempS.m_skinningMatrices.data(), inputMesh, tempM);
			for (uint32_t vi = 0; vi < tempM.vertexCount; vi++)
			{
				const glm::vec3* vertexPos = tempM.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, vi);
				float distSq = glm::distance2(outputSphere.center, *vertexPos);
				if (distSq > maxDistanceSq)
					maxDistanceSq = distSq;
			}
		}
	}
	outputSphere.radius = glm::sqrt(maxDistanceSq);
	tempM.Destroy();
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
			}
		}
	}
}