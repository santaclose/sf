#include "MeshProcessor.h"

#include <iostream>
#include <fstream>

#include <Random.h>
#include <Math.hpp>
#include <aobaker.h>

void sf::MeshProcessor::ComputeNormals(MeshData& mesh, bool normalize)
{
	for (Vertex& v : mesh.vertexVector)
		v.normal = { 0.0f,0.0f,0.0f };

	for (int i = 0; i < mesh.indexVector.size(); i += 3)
	{
		glm::vec3 a = mesh.vertexVector[mesh.indexVector[i + 1]].position - mesh.vertexVector[mesh.indexVector[i + 0]].position;
		glm::vec3 b = mesh.vertexVector[mesh.indexVector[i + 2]].position - mesh.vertexVector[mesh.indexVector[i + 0]].position;
		glm::vec3 n = glm::normalize(glm::cross(a, b));

		mesh.vertexVector[mesh.indexVector[i + 0]].normal += n;
		mesh.vertexVector[mesh.indexVector[i + 1]].normal += n;
		mesh.vertexVector[mesh.indexVector[i + 2]].normal += n;
	}

	if (normalize)
	{
		for (Vertex& v : mesh.vertexVector)
			v.normal = glm::normalize(v.normal);
	}
}

void sf::MeshProcessor::ComputeTangentSpace(MeshData& mesh)
{
	for (int i = 0; i < mesh.indexVector.size(); i += 3)
	{
		unsigned int ci = mesh.indexVector[i + 0];
		unsigned int ni = mesh.indexVector[i + 1];
		unsigned int nni = mesh.indexVector[i + 2];

		glm::vec3 edge1 = mesh.vertexVector[ni].position - mesh.vertexVector[ci].position;
		glm::vec3 edge2 = mesh.vertexVector[nni].position - mesh.vertexVector[ci].position;
		glm::vec2 deltaUV1 = mesh.vertexVector[ni].textureCoord - mesh.vertexVector[ci].textureCoord;
		glm::vec2 deltaUV2 = mesh.vertexVector[nni].textureCoord - mesh.vertexVector[ci].textureCoord;

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

		mesh.vertexVector[ci].tangent += t;
		mesh.vertexVector[ni].tangent += t;
		mesh.vertexVector[nni].tangent += t;

		mesh.vertexVector[ci].bitangent += b;
		mesh.vertexVector[ni].bitangent += b;
		mesh.vertexVector[nni].bitangent += b;
	}
}

void sf::MeshProcessor::BakeAoToVertices(MeshData& mesh)
{
	aobaker::config conf;
	aobaker::BakeAoToVertices(&mesh.vertexVector[0].position.x, &mesh.vertexVector[0].extraData.x, mesh.vertexVector.size(), sizeof(Vertex), sizeof(Vertex),
		&mesh.indexVector[0], mesh.indexVector.size(), conf);
}