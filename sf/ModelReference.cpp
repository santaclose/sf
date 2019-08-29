#include "ModelReference.h"
#include "Camera.h"

std::vector<ModelReference*> ModelReference::modelReferences;

void ModelReference::SendMatrixToShader()
{
	if (m_matrixUpdatePending)
		UpdateTransformMatrix();
	m_material->m_shader->SetUniformMatrix4fv("modelMatrix", &m_transformMatrix[0][0]);
}

void ModelReference::CreateFomModel(const Model& theModel)
{
	m_gl_vertexBuffer = theModel.m_gl_vertexBuffer;
	m_gl_indexBuffer = theModel.m_gl_indexBuffer;
	m_indexCount = theModel.m_indexVector.size();
	m_material = theModel.m_material;

	modelReferences.push_back(this);
}

void ModelReference::SetMaterial(Material* theMaterial)
{
	m_material = theMaterial;
}

void ModelReference::Draw()
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

	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
}

void ModelReference::DrawAll()
{
	for (ModelReference* m : modelReferences)
	{
		m->Draw();
	}
	//glBufferData(GL_ARRAY_BUFFER, vertexVector.size() * sizeof(Vertex), &vertexVector[0], GL_DYNAMIC_DRAW);						// update vertices
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexVector.size() * sizeof(unsigned int), &indexVector[0], GL_DYNAMIC_DRAW);		// update indices to draw

	//glDrawElements(GL_TRIANGLES, indexVector.size(), GL_UNSIGNED_INT, nullptr);
}