#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Vertex.h"
#include "Object.h"
#include "Material.h"

class ModelReference;

class Model : public Object
{
	friend ModelReference;

	static std::vector<Model*> models;

	void SendMatrixToShader();

	unsigned int m_gl_vertexBuffer;
	unsigned int m_gl_indexBuffer;
	std::vector<Vertex> m_vertexVector;
	std::vector<unsigned int> m_indexVector;

	Material* m_material;

public:
	~Model();

	void CreatePlane(float size);
	void CreateCube(float size);

	void CreateFromOBJ(const std::string& filePath, float size = 1.0, bool faceted = false);

	void SetMaterial(Material* theMaterial);

	void Draw();
	static void DrawAll();
};

