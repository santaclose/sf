#include "MeshReference.h"

#include <Camera.h>

void sf::MeshReference::SendMatrixToShader(Material& material)
{
	if (m_matrixUpdatePending)
		UpdateTransformMatrix();
	material.m_shader->SetUniformMatrix4fv("modelMatrix", &m_transformMatrix[0][0]);
}

void sf::MeshReference::CreateFomMesh(Mesh& theMesh)
{
	m_originalModel = &theMesh;
	m_originalModel->m_references.push_back(this);
}