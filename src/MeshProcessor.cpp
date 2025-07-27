#include "MeshProcessor.h"

#include <ml.h>

#include <iostream>
#include <fstream>
#include <cstring>

#include <Random.h>
#include <Geometry.h>

namespace sf {

	template<typename PDT, typename NDT> // position data type and normal data type
	void ComputeNormalsT(MeshData& mesh, bool normalize = false)
	{
		// set all normals to zero
		for (uint32_t i = 0; i < mesh.vertexCount; i++)
		{
			NDT* targetPointer = (NDT*)mesh.AccessVertexComponent(BufferComponent::VertexNormal, i);
			targetPointer->x = targetPointer->y = targetPointer->z = 0.0;
		}

		// compute normals from face vertex positions
		for (int i = 0; i < mesh.indexVector.size(); i += 3)
		{
			NDT faceNormal;
			PDT* a = (PDT*)mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[i + 0]);
			PDT* b = (PDT*)mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[i + 1]);
			PDT* c = (PDT*)mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[i + 2]);
			NDT ab = (*b) - (*a);
			NDT ac = (*c) - (*a);
			faceNormal = glm::normalize(glm::cross(ab, ac));
			NDT* normalA = (NDT*)mesh.AccessVertexComponent(BufferComponent::VertexNormal, mesh.indexVector[i + 0]);
			NDT* normalB = (NDT*)mesh.AccessVertexComponent(BufferComponent::VertexNormal, mesh.indexVector[i + 1]);
			NDT* normalC = (NDT*)mesh.AccessVertexComponent(BufferComponent::VertexNormal, mesh.indexVector[i + 2]);
			*normalA += faceNormal;
			*normalB += faceNormal;
			*normalC += faceNormal;
		}

		if (normalize)
		{
			for (uint32_t i = 0; i < mesh.vertexCount; i++)
			{
				NDT* targetPointer = (NDT*)mesh.AccessVertexComponent(BufferComponent::VertexNormal, i);
				*targetPointer = glm::normalize(*targetPointer);
			}
		}
	}

	template<typename PDT, typename TDT, typename UDT>
	void ComputeTangentSpaceT(MeshData& mesh)
	{
		// set all tangents to zero
		for (uint32_t i = 0; i < mesh.vertexCount; i++)
		{
			TDT* targetPointer;
			targetPointer = (TDT*)mesh.AccessVertexComponent(BufferComponent::VertexTangent, i);
			targetPointer->x = targetPointer->y = targetPointer->z = 0.0;
		}

		for (int i = 0; i < mesh.indexVector.size(); i += 3)
		{
			PDT* ap = (PDT*)mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[i + 0]);
			PDT* bp = (PDT*)mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[i + 1]);
			PDT* cp = (PDT*)mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[i + 2]);
			UDT* au = (UDT*)mesh.AccessVertexComponent(BufferComponent::VertexUV, mesh.indexVector[i + 0]);
			UDT* bu = (UDT*)mesh.AccessVertexComponent(BufferComponent::VertexUV, mesh.indexVector[i + 1]);
			UDT* cu = (UDT*)mesh.AccessVertexComponent(BufferComponent::VertexUV, mesh.indexVector[i + 2]);
			TDT* at = (TDT*)mesh.AccessVertexComponent(BufferComponent::VertexTangent, mesh.indexVector[i + 0]);
			TDT* bt = (TDT*)mesh.AccessVertexComponent(BufferComponent::VertexTangent, mesh.indexVector[i + 1]);
			TDT* ct = (TDT*)mesh.AccessVertexComponent(BufferComponent::VertexTangent, mesh.indexVector[i + 2]);

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

void sf::MeshProcessor::ComputeNormals(MeshData& mesh, bool normalize)
{
	DataType positionDataType = mesh.vertexBufferLayout.GetComponentInfo(BufferComponent::VertexPosition)->dataType;
	DataType normalDataType = mesh.vertexBufferLayout.GetComponentInfo(BufferComponent::VertexNormal)->dataType;

	assert(positionDataType == DataType::vec3f32 || positionDataType == DataType::vec3f64);
	assert(normalDataType == DataType::vec3f32 || normalDataType == DataType::vec3f64);

	if (positionDataType == DataType::vec3f32)
	{
		if (normalDataType == DataType::vec3f32)
			ComputeNormalsT<glm::vec3, glm::vec3>(mesh, normalize);
		else
			ComputeNormalsT<glm::vec3, glm::dvec3>(mesh, normalize);
	}
	else
	{
		if (normalDataType == DataType::vec3f32)
			ComputeNormalsT<glm::dvec3, glm::vec3>(mesh, normalize);
		else
			ComputeNormalsT<glm::dvec3, glm::dvec3>(mesh, normalize);
	}
}

void sf::MeshProcessor::ComputeTangentSpace(MeshData& mesh)
{
	DataType positionDataType = mesh.vertexBufferLayout.GetComponentInfo(BufferComponent::VertexPosition)->dataType;
	DataType uvsDataType = mesh.vertexBufferLayout.GetComponentInfo(BufferComponent::VertexUV)->dataType;
	DataType tangentDataType = mesh.vertexBufferLayout.GetComponentInfo(BufferComponent::VertexTangent)->dataType;

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
	DataType positionDataType = mesh.vertexBufferLayout.GetComponentInfo(BufferComponent::VertexPosition)->dataType;
	DataType aoDataType = mesh.vertexBufferLayout.GetComponentInfo(BufferComponent::VertexAO)->dataType;

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
			minvpos = maxvpos = *((glm::vec3*) mesh.AccessVertexComponent(BufferComponent::VertexPosition, 0));
			for (int i = 1; i < mesh.vertexCount; i++)
			{
				glm::vec3 vertexPos = *((glm::vec3*) mesh.AccessVertexComponent(BufferComponent::VertexPosition, i));
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
		glm::vec3 vertexPos = *((glm::vec3*) mesh.AccessVertexComponent(BufferComponent::VertexPosition, q));
		float* aoTarget = (float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, q);

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
				for (int j = 0; j < mesh.indexVector.size() && !didHit; j += 3) // for each face, intersect
				{
					if (mesh.indexVector[j + 0] == q || mesh.indexVector[j + 1] == q || mesh.indexVector[j + 2] == q)
						continue; // current vertex belongs to this face

					didHit = Geometry::IntersectRayTriangle(vertexPos + (rayDir * config->rayOriginOffset), rayDir,
						*((glm::vec3*) mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[j + 0])),
						*((glm::vec3*) mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[j + 1])),
						*((glm::vec3*) mesh.AccessVertexComponent(BufferComponent::VertexPosition, mesh.indexVector[j + 2])),
						&distance);
				}
				if (distance > config->rayDistance)
					distance = config->rayDistance;
			}
		}
		*aoTarget = ComputeOcclusion(rayResults, config->rayDistance, config->falloff);
	}

	for (int pass = 0; pass < config->denoisePasses; pass++)
	{
		for (int i = 0; i < mesh.indexVector.size(); i += 3)
		{
			float average =
				(*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 0])) +
					*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 1])) +
					*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 2]))) / 3.0f;

			*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 0])) = glm::mix(*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 0])), average, config->denoiseWeight);
			*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 1])) = glm::mix(*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 1])), average, config->denoiseWeight);
			*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 2])) = glm::mix(*((float*) mesh.AccessVertexComponent(BufferComponent::VertexAO, mesh.indexVector[i + 2])), average, config->denoiseWeight);
		}
	}
}

void sf::MeshProcessor::GenerateMeshWithFunction(MeshData& mesh, void(*functionPointer)())
{
	sfmg::ml::Initialize(true);
	functionPointer();
	uint32_t vertexCount, indexCount;
	const uint32_t* indices;
	sfmg::ml::Vertex* vertices;
	sfmg::ml::GetData(vertices, vertexCount, indices, indexCount);

	mesh.pieces.clear();
	mesh.pieces.push_back(0);
	mesh.vertexBufferLayout = BufferLayout({
		BufferComponent::VertexPosition,
		BufferComponent::VertexNormal,
		BufferComponent::VertexUV });
	uint32_t vertexSizeInBytes = mesh.vertexBufferLayout.GetSize();
	mesh.indexVector.resize(indexCount);
	memcpy(&(mesh.indexVector[0]), indices, sizeof(uint32_t) * indexCount);
	mesh.vertexBuffer = malloc(vertexSizeInBytes * vertexCount);
	memcpy(mesh.vertexBuffer, vertices, vertexSizeInBytes * vertexCount);
	mesh.vertexCount = vertexCount;
}
