#include "Model.h"
#include "ModelPrimitives.inl"
#include "ModelLoader.h"
#include "Camera.h"
#include "Texture.h"
#include <iostream>

std::vector<Model*> Model::models;

void Model::SendMatrixToShader(Shader& theShader)
{
	if (m_matrixUpdatePending)
		UpdateTransformMatrix();
	theShader.SetUniformMatrix4fv("modelMatrix", &m_transformMatrix[0][0]);//&m_transformMatrix[0][0]);
}

void Model::UpdateTransformMatrix()
{
	//std::cout << "Updating model matrix\n";

	m_transformMatrix = glm::translate(glm::mat4(1.0), m_position);

	glm::mat4 rotationMatrix = (glm::mat4) m_rotation;
	m_transformMatrix *= rotationMatrix;

	m_matrixUpdatePending = false;
}

Model::~Model()
{
	m_indexVector.clear();
	m_vertexVector.clear();
}

void Model::CreatePlane(float size)
{
	m_transformMatrix = glm::mat4(1.0);

	ModelPrimitives::Plane(m_vertexVector, m_indexVector, size);

	glGenBuffers(1, &m_gl_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);						// update vertices
	glGenBuffers(1, &m_gl_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);		// update indices to draw

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3)); // normal
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // color
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 10)); // texture coord
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	//glEnableVertexAttribArray(3);

	models.push_back(this);
}

void Model::CreateCube(float size)
{
	m_transformMatrix = glm::mat4(1.0);

	ModelPrimitives::Cube(m_vertexVector, m_indexVector, size);

	glGenBuffers(1, &m_gl_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);						// update vertices
	glGenBuffers(1, &m_gl_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);		// update indices to draw

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3)); // normal
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // texture coord
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // color
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 10)); // texture coord
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	//glEnableVertexAttribArray(3);

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
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // texture coord
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // color
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 10)); // texture coord
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	//glEnableVertexAttribArray(3);

	models.push_back(this);
}

void Model::SetShader(Shader* theShader)
{
	m_shader = theShader;
}

void Model::draw()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3)); // normal
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6)); // texture coord
	m_shader->Bind();

	Camera::boundCamera->SendMatrixToShader(*m_shader);
	SendMatrixToShader(*m_shader);

	glDrawElements(GL_TRIANGLES, m_indexVector.size(), GL_UNSIGNED_INT, nullptr);
}

void Model::drawAll()
{
	for (Model* m : models)
	{
		m->draw();
	}
	//glBufferData(GL_ARRAY_BUFFER, vertexVector.size() * sizeof(Vertex), &vertexVector[0], GL_DYNAMIC_DRAW);						// update vertices
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexVector.size() * sizeof(unsigned int), &indexVector[0], GL_DYNAMIC_DRAW);		// update indices to draw

	//glDrawElements(GL_TRIANGLES, indexVector.size(), GL_UNSIGNED_INT, nullptr);
}