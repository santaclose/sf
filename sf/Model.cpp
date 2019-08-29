#include "Model.h"
#include "ModelPrimitives.inl"
#include "ModelLoader.h"
#include "Camera.h"
#include "Texture.h"
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

void Model::CreatePlane(float size)
{
	ModelPrimitives::Plane(m_vertexVector, m_indexVector, size);

	glGenBuffers(1, &m_gl_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);						// update vertices
	glGenBuffers(1, &m_gl_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);		// update indices to draw

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3)); // normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // tangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9)); // bitangent
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 12)); // texture coords

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	models.push_back(this);
}

void Model::CreateCube(float size)
{
	ModelPrimitives::Cube(m_vertexVector, m_indexVector, size);

	glGenBuffers(1, &m_gl_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);						// update vertices
	glGenBuffers(1, &m_gl_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);		// update indices to draw

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3)); // normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // tangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9)); // bitangent
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 12)); // texture coords

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	models.push_back(this);
}
void Model::CreateFromOBJ(const std::string& filePath, float size, bool faceted)
{
	if (faceted)
		ModelLoader::LoadFacetedOBJFile(m_vertexVector, m_indexVector, filePath, size);
	else
		ModelLoader::LoadOBJFile(m_vertexVector, m_indexVector, filePath, size);

	glGenBuffers(1, &m_gl_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);						// update vertices
	glGenBuffers(1, &m_gl_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);		// update indices to draw

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3)); // normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // tangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9)); // bitangent
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 12)); // texture coords

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	models.push_back(this);
}

void Model::SetMaterial(Material* theMaterial)
{
	m_material = theMaterial;
}

void Model::Draw()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3)); // normal
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // tangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9)); // bitangent
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 12)); // texture coords
	//m_shader->Bind();
	m_material->Bind();

	Camera::SendMatrixToShader(*(m_material->m_shader));
	SendMatrixToShader();

	glDrawElements(GL_TRIANGLES, m_indexVector.size(), GL_UNSIGNED_INT, nullptr);
}

void Model::DrawAll()
{
	for (Model* m : models)
	{
		m->Draw();
	}
	//glBufferData(GL_ARRAY_BUFFER, vertexVector.size() * sizeof(Vertex), &vertexVector[0], GL_DYNAMIC_DRAW);						// update vertices
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexVector.size() * sizeof(unsigned int), &indexVector[0], GL_DYNAMIC_DRAW);		// update indices to draw

	//glDrawElements(GL_TRIANGLES, indexVector.size(), GL_UNSIGNED_INT, nullptr);
}