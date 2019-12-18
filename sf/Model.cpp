#include "Model.h"
#include "ModelReference.h"
#include "ModelLoader.h"
#include "Camera.h"
#include "Texture.h"
#include "ofv/ml.h"
#include <iostream>

std::vector<Model*> Model::models;

void Model::SendMatrixToShader()
{
	if (m_matrixUpdatePending)
		UpdateTransformMatrix();
	m_material->m_shader->SetUniformMatrix4fv("modelMatrix", &m_transformMatrix[0][0]);
}

Model::~Model()
{
	m_indexVector.clear();
	m_vertexVector.clear();
}

// obj should be treated by other code and here load an engine format instead
void Model::CreateFromOBJ(const std::string& filePath, float size, bool faceted)
{
	if (faceted)
		ModelLoader::LoadFacetedOBJFile(m_vertexVector, m_indexVector, filePath, size);
	else
		ModelLoader::LoadOBJFile(m_vertexVector, m_indexVector, filePath, size);

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

void Model::CreateFromCode(void (*generateModelFunc)(), bool smooth)
{
	ofv::ml::Initialize(smooth, &m_vertexVector, &m_indexVector);
	generateModelFunc();

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

void Model::SetMaterial(Material* theMaterial)
{
	m_material = theMaterial;
}

void Model::Draw()
{
	m_material->Bind();

	Camera::SendMatrixToShader(*(m_material->m_shader));
	SendMatrixToShader();

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