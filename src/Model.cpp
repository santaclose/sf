#include "Model.h"
#include "ModelReference.h"
#include "GltfController.h"
#include "ObjController.h"
#include "Camera.h"
#include "Texture.h"
#include "Random.h"
#include "Math.h"

#include <ml.h>
#include <iostream>

namespace sf {

	std::vector<Model*> sf::Model::models;
}

void sf::Model::SendMatrixToShader()
{
	if (m_matrixUpdatePending)
		UpdateTransformMatrix();
	m_material->m_shader->SetUniformMatrix4fv("modelMatrix", &m_transformMatrix[0][0]);
}
void sf::Model::CompleteFromVectors()
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
	glEnableVertexAttribArray(5); // extra data
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 14));

	glBindVertexArray(0);

	models.push_back(this);
}
void sf::Model::ReloadVertexData()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);
}

void sf::Model::CreateFromGltf(unsigned int gltfID, unsigned int meshIndex)
{
	GltfController::GetModel(gltfID, meshIndex, m_vertexVector, m_indexVector);
	CompleteFromVectors();
}
void sf::Model::CreateFromObj(unsigned int objID, unsigned int meshIndex)
{
	ObjController::GetModel(objID, meshIndex, m_vertexVector, m_indexVector);
	CompleteFromVectors();
}
void sf::Model::CreateFromCode(void (*generateModelFunc)(), bool smooth)
{
	sfmg::ml::Initialize(smooth, &m_vertexVector, &m_indexVector);
	generateModelFunc();
	CompleteFromVectors();
}

void sf::Model::SetMaterial(Material* theMaterial)
{
	m_material = theMaterial;
}
void sf::Model::Draw()
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
void sf::Model::DrawAll()
{
	for (Model* m : models)
	{
		m->Draw();
	}
}