#include "MeshProcessor.h"

#include <ml.h>

#include <iostream>
#include <fstream>

#include <Random.h>
#include <Math.hpp>
#include <aobaker.h>

namespace sf {

	template<typename PDT, typename NDT> // position data type and normal data type
	void ComputeNormalsT(MeshData& mesh, bool normalize = false)
	{
		// set all normals to zero
		for (uint32_t i = 0; i < mesh.vertexCount; i++)
		{
			NDT* targetPointer = (NDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Normal, i);
			targetPointer->x = targetPointer->y = targetPointer->z = 0.0;
		}

		// compute normals from face vertex positions
		for (int i = 0; i < mesh.indexVector.size(); i += 3)
		{
			NDT faceNormal;
			PDT* a = (PDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, mesh.indexVector[i + 0]);
			PDT* b = (PDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, mesh.indexVector[i + 1]);
			PDT* c = (PDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, mesh.indexVector[i + 2]);
			NDT ab = (*b) - (*a);
			NDT ac = (*c) - (*a);
			faceNormal = glm::normalize(glm::cross(ab, ac));
			NDT* normalA = (NDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Normal, mesh.indexVector[i + 0]);
			NDT* normalB = (NDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Normal, mesh.indexVector[i + 1]);
			NDT* normalC = (NDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Normal, mesh.indexVector[i + 2]);
			*normalA += faceNormal;
			*normalB += faceNormal;
			*normalC += faceNormal;
		}

		if (normalize)
		{
			for (uint32_t i = 0; i < mesh.vertexCount; i++)
			{
				NDT* targetPointer = (NDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Normal, i);
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
			targetPointer = (TDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Tangent, i);
			targetPointer->x = targetPointer->y = targetPointer->z = 0.0;
			targetPointer = (TDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Bitangent, i);
			targetPointer->x = targetPointer->y = targetPointer->z = 0.0;
		}

		for (int i = 0; i < mesh.indexVector.size(); i += 3)
		{
			PDT* ap = (PDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, mesh.indexVector[i + 0]);
			PDT* bp = (PDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, mesh.indexVector[i + 1]);
			PDT* cp = (PDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, mesh.indexVector[i + 2]);
			UDT* au = (UDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::TexCoords, mesh.indexVector[i + 0]);
			UDT* bu = (UDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::TexCoords, mesh.indexVector[i + 1]);
			UDT* cu = (UDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::TexCoords, mesh.indexVector[i + 2]);
			TDT* at = (TDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Tangent, mesh.indexVector[i + 0]);
			TDT* bt = (TDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Tangent, mesh.indexVector[i + 1]);
			TDT* ct = (TDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Tangent, mesh.indexVector[i + 2]);
			TDT* ab = (TDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Bitangent, mesh.indexVector[i + 0]);
			TDT* bb = (TDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Bitangent, mesh.indexVector[i + 1]);
			TDT* cb = (TDT*)mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Bitangent, mesh.indexVector[i + 2]);

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
			*ab += b;
			*bb += b;
			*cb += b;
		}
	}
}

void sf::MeshProcessor::ComputeNormals(MeshData& mesh, bool normalize)
{
	DataType positionDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Position)->dataType;
	DataType normalDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Normal)->dataType;

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
	DataType positionDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Position)->dataType;
	DataType uvsDataType = mesh.vertexLayout.GetComponent(VertexAttribute::TexCoords)->dataType;
	DataType tangentDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Tangent)->dataType;
	DataType bitangentDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Bitangent)->dataType;

	assert(positionDataType == DataType::vec3f32 || positionDataType == DataType::vec3f64);
	assert(uvsDataType == DataType::vec2f32 || uvsDataType == DataType::vec2f64);
	assert(tangentDataType == DataType::vec3f32 || tangentDataType == DataType::vec3f64);
	assert(bitangentDataType == tangentDataType);

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

void sf::MeshProcessor::BakeAoToVertices(MeshData& mesh)
{
	DataType positionDataType = mesh.vertexLayout.GetComponent(VertexAttribute::Position)->dataType;
	DataType aoDataType = mesh.vertexLayout.GetComponent(VertexAttribute::AmbientOcclusion)->dataType;

	assert(positionDataType == DataType::vec3f32);
	assert(aoDataType == DataType::f32);

	float* posPointer = (float*) mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::Position, 0);
	float* aoPointer = (float*) mesh.vertexLayout.Access(mesh.vertexBuffer, VertexAttribute::AmbientOcclusion, 0);

	uint32_t vertexSizeInBytes = mesh.vertexLayout.GetSize();

	aobaker::config conf;
	aobaker::BakeAoToVertices(posPointer, aoPointer, mesh.vertexCount, vertexSizeInBytes, vertexSizeInBytes, &mesh.indexVector[0], mesh.indexVector.size(), conf);
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
	mesh.vertexLayout = DataLayout({
		{VertexAttribute::Position, DataType::vec3f32},
		{VertexAttribute::Normal, DataType::vec3f32},
		{VertexAttribute::TexCoords, DataType::vec2f32} });
	uint32_t vertexSizeInBytes = mesh.vertexLayout.GetSize();
	mesh.indexVector.resize(indexCount);
	memcpy(&(mesh.indexVector[0]), indices, sizeof(uint32_t) * indexCount);
	mesh.vertexBuffer = malloc(vertexSizeInBytes * vertexCount);
	memcpy(mesh.vertexBuffer, vertices, vertexSizeInBytes * vertexCount);
	mesh.vertexCount = vertexCount;
}
