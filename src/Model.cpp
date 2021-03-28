#include "Model.h"
#include "ModelReference.h"
#include "GltfController.h"
#include "Camera.h"
#include "Texture.h"
#include "Random.h"
#include "Math.h"

#include <ml.h>
#include <iostream>

std::vector<Model*> Model::models;

void Model::SendMatrixToShader()
{
	if (m_matrixUpdatePending)
		UpdateTransformMatrix();
	m_material->m_shader->SetUniformMatrix4fv("modelMatrix", &m_transformMatrix[0][0]);
}

void Model::CompleteFromVectors()
{
	glGenVertexArrays(1, &m_gl_vao);
	glGenBuffers(1, &m_gl_vertexBuffer);
	glGenBuffers(1, &m_gl_indexBuffer);

	glBindVertexArray(m_gl_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);

	// update vertices
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	// update indices to draw
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1); // normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(2); // tangent
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(3); // bitangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9));
	glEnableVertexAttribArray(4); // texture coords
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 12));

	glBindVertexArray(0);

	models.push_back(this);
}

void Model::ReloadVertexData()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);
}

void Model::CreateFromGltf(unsigned int gltfID, unsigned int meshIndex)
{
	GltfController::Model(gltfID, meshIndex, m_vertexVector, m_indexVector);
	CompleteFromVectors();
}

void Model::CreateFromCode(void (*generateModelFunc)(), bool smooth)
{
	sfmg::ml::Initialize(smooth, &m_vertexVector, &m_indexVector);
	generateModelFunc();
	CompleteFromVectors();
}

void Model::BakeAoToVertices(int rayCount)
{
	int maxHitCount = 0;
	#pragma omp parallel for
	for (int q = 0; q < m_vertexVector.size(); q++)
	{
		int hitCounter = 0;
		Vertex& v = m_vertexVector[q];

		for (int i = 0; i < rayCount; i++)
		{
			float alpha = Random::Float() * 2.0f * Math::Pi;
			float beta = Random::Float() * 2.0f * Math::Pi;

			glm::vec3 rayDir;
			//rayDir = glm::vec3(0.0f, 1.0f, 0.0f);
			rayDir.x = glm::cos(alpha) * glm::cos(beta);
			rayDir.z = glm::sin(alpha) * glm::cos(beta);
			//rayDir.y = glm::sin(beta);
			rayDir.y = glm::abs(glm::sin(beta));

			bool didHit = false;
			for (int j = 0; j < m_indexVector.size() && !didHit; j += 3) // for each face, intersect
			{
				if (m_indexVector[j + 0] == q || m_indexVector[j + 1] == q || m_indexVector[j + 2] == q)
					continue; // current vertex belongs to this face

				glm::vec3 faceNormal = glm::normalize(glm::cross(
					m_vertexVector[m_indexVector[j + 1]].position - m_vertexVector[m_indexVector[j + 0]].position,
					m_vertexVector[m_indexVector[j + 2]].position - m_vertexVector[m_indexVector[j + 0]].position));
				if (glm::dot(rayDir, faceNormal) < 0)
					continue;

				didHit = Math::RayTriIntersect(v.position, rayDir,
					m_vertexVector[m_indexVector[j + 0]].position,
					m_vertexVector[m_indexVector[j + 1]].position,
					m_vertexVector[m_indexVector[j + 2]].position);
			}
			if (didHit)
				hitCounter++;
		}
		v.textureCoord.x = (float)hitCounter;
		if (hitCounter > maxHitCount)
			maxHitCount = hitCounter;
	}

	for (Vertex& v : m_vertexVector)
	{
		v.textureCoord.x = 1.0f - (v.textureCoord.x / (float)maxHitCount);
	}
	std::cout << "[Model] Finished baking ao to vertices\n";
}

void Model::SetMaterial(Material* theMaterial)
{
	m_material = theMaterial;
}

void Model::Draw()
{
	m_material->Bind();

	m_material->m_shader->SetUniformMatrix4fv("cameraMatrix", &(Camera::GetMatrix()[0][0]));
	m_material->m_shader->SetUniform3fv("camPos", &(Camera::boundCamera->GetPosition()[0]));

	SendMatrixToShader();
	
	//std::cout << "model being drawn\n";
	glBindVertexArray(m_gl_vao);
	glDrawElements(GL_TRIANGLES, m_indexVector.size(), GL_UNSIGNED_INT, nullptr);
	for (ModelReference* m : m_references)
	{
		// replace model matrix and draw again
		m->SendMatrixToShader();
		glDrawElements(GL_TRIANGLES, m_indexVector.size(), GL_UNSIGNED_INT, nullptr);
	}
	glBindVertexArray(0);
}

void Model::DrawAll()
{
	for (Model* m : models)
	{
		m->Draw();
	}
}