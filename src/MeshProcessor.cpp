#include "MeshProcessor.h"

#include <iostream>

#include <Random.h>
#include <Math.hpp>
#include <aobaker.h>

void sf::MeshProcessor::ComputeNormals(Mesh& mesh, bool normalize)
{
	for (Vertex& v : mesh.m_vertexVector)
		v.normal = { 0.0f,0.0f,0.0f };

	for (int i = 0; i < mesh.m_indexVector.size(); i += 3)
	{
		glm::vec3 a = mesh.m_vertexVector[mesh.m_indexVector[i + 1]].position - mesh.m_vertexVector[mesh.m_indexVector[i + 0]].position;
		glm::vec3 b = mesh.m_vertexVector[mesh.m_indexVector[i + 2]].position - mesh.m_vertexVector[mesh.m_indexVector[i + 0]].position;
		glm::vec3 n = glm::normalize(glm::cross(a, b));

		mesh.m_vertexVector[mesh.m_indexVector[i + 0]].normal += n;
		mesh.m_vertexVector[mesh.m_indexVector[i + 1]].normal += n;
		mesh.m_vertexVector[mesh.m_indexVector[i + 2]].normal += n;
	}

	if (normalize)
	{
		for (Vertex& v : mesh.m_vertexVector)
			v.normal = glm::normalize(v.normal);
	}
}

void sf::MeshProcessor::ComputeTangentSpace(Mesh& mesh)
{
	for (int i = 0; i < mesh.m_indexVector.size(); i += 3)
	{
		unsigned int ci = mesh.m_indexVector[i + 0];
		unsigned int ni = mesh.m_indexVector[i + 1];
		unsigned int nni = mesh.m_indexVector[i + 2];

		glm::vec3 edge1 = mesh.m_vertexVector[ni].position - mesh.m_vertexVector[ci].position;
		glm::vec3 edge2 = mesh.m_vertexVector[nni].position - mesh.m_vertexVector[ci].position;
		glm::vec2 deltaUV1 = mesh.m_vertexVector[ni].textureCoord - mesh.m_vertexVector[ci].textureCoord;
		glm::vec2 deltaUV2 = mesh.m_vertexVector[nni].textureCoord - mesh.m_vertexVector[ci].textureCoord;

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

		mesh.m_vertexVector[ci].tangent += t;
		mesh.m_vertexVector[ni].tangent += t;
		mesh.m_vertexVector[nni].tangent += t;

		mesh.m_vertexVector[ci].bitangent += b;
		mesh.m_vertexVector[ni].bitangent += b;
		mesh.m_vertexVector[nni].bitangent += b;
	}
}

void sf::MeshProcessor::BakeAoToVertices(Mesh& mesh)
{
	aobaker::config conf;
	aobaker::BakeAoToVertices(&mesh.m_vertexVector[0].position.x, &mesh.m_vertexVector[0].extraData.x, mesh.m_vertexVector.size(), sizeof(Vertex), sizeof(Vertex),
		&mesh.m_indexVector[0], mesh.m_indexVector.size(), conf);
}