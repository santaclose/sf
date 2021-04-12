#include "ModelProcessor.h"

#include <iostream>

#include <Random.h>
#include <Math.hpp>
#include <aobaker.h>

void sf::ModelProcessor::ComputeNormals(Model& model, bool normalize)
{
	for (Vertex& v : model.m_vertexVector)
		v.normal = { 0.0f,0.0f,0.0f };

	for (int i = 0; i < model.m_indexVector.size(); i += 3)
	{
		glm::vec3 a = model.m_vertexVector[model.m_indexVector[i + 1]].position - model.m_vertexVector[model.m_indexVector[i + 0]].position;
		glm::vec3 b = model.m_vertexVector[model.m_indexVector[i + 2]].position - model.m_vertexVector[model.m_indexVector[i + 0]].position;
		glm::vec3 n = glm::normalize(glm::cross(a, b));

		model.m_vertexVector[model.m_indexVector[i + 0]].normal += n;
		model.m_vertexVector[model.m_indexVector[i + 1]].normal += n;
		model.m_vertexVector[model.m_indexVector[i + 2]].normal += n;
	}

	if (normalize)
	{
		for (Vertex& v : model.m_vertexVector)
			v.normal = glm::normalize(v.normal);
	}
}

void sf::ModelProcessor::ComputeTangentSpace(Model& model)
{
	for (int i = 0; i < model.m_indexVector.size(); i += 3)
	{
		unsigned int ci = model.m_indexVector[i + 0];
		unsigned int ni = model.m_indexVector[i + 1];
		unsigned int nni = model.m_indexVector[i + 2];

		glm::vec3 edge1 = model.m_vertexVector[ni].position - model.m_vertexVector[ci].position;
		glm::vec3 edge2 = model.m_vertexVector[nni].position - model.m_vertexVector[ci].position;
		glm::vec2 deltaUV1 = model.m_vertexVector[ni].textureCoord - model.m_vertexVector[ci].textureCoord;
		glm::vec2 deltaUV2 = model.m_vertexVector[nni].textureCoord - model.m_vertexVector[ci].textureCoord;

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

		model.m_vertexVector[ci].tangent += t;
		model.m_vertexVector[ni].tangent += t;
		model.m_vertexVector[nni].tangent += t;

		model.m_vertexVector[ci].bitangent += b;
		model.m_vertexVector[ni].bitangent += b;
		model.m_vertexVector[nni].bitangent += b;
	}
}

void sf::ModelProcessor::BakeAoToVertices(Model& model)
{
	aobaker::config conf;
	aobaker::BakeAoToVertices(&model.m_vertexVector[0].position.x, &model.m_vertexVector[0].extraData.x, model.m_vertexVector.size(), sizeof(Vertex), sizeof(Vertex),
		&model.m_indexVector[0], model.m_indexVector.size(), conf);
}