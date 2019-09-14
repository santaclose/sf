#pragma once
#include "Object.h"
#include "Material.h"
#include "Model.h"

// a model reference only shares vertex and index buffers
class ModelReference : public Object
{
	friend Model;

	Model* m_originalModel;
	void SendMatrixToShader();

public:
	//ModelReference(const Model& theModel);
	void CreateFomModel(Model& theModel);
};