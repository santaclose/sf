#pragma once
#include "Object.h"
#include "Material.h"
#include "Model.h"

// a model reference only shares vertex and index buffers
class ModelReference : public Object
{
	friend Model;
	static std::vector<ModelReference*> modelReferences;

	void SendMatrixToShader();

	unsigned int m_gl_vertexBuffer;
	unsigned int m_gl_indexBuffer;
	unsigned int m_indexCount;

	Material* m_material;

public:
	void CreateFomModel(const Model& theModel);

	void SetMaterial(Material* theMaterial);

	void Draw();
	static void DrawAll();
};

