// stores a matrix for model duplicates doesnt contain vertices or materials

#include "ModelReference.h"
#include "Camera.h"

void ModelReference::SendMatrixToShader()
{
	if (m_matrixUpdatePending)
		UpdateTransformMatrix();
	m_originalModel->m_material->m_shader->SetUniformMatrix4fv("modelMatrix", &m_transformMatrix[0][0]);
}

void ModelReference::CreateFomModel(Model& theModel)
{
	m_originalModel = &theModel;
	m_originalModel->m_references.push_back(this);
}