#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Vertex.h"
#include "Entity.h"
#include "Material.h"

class Model : public Entity
{
	static std::vector<Model*> models;

	void SendMatrixToShader();
	void UpdateTransformMatrix();

	unsigned int m_gl_vertexBuffer;
	unsigned int m_gl_indexBuffer;
	std::vector<Vertex> m_vertexVector;
	std::vector<unsigned int> m_indexVector;
	glm::mat4 m_transformMatrix;

	//Shader* m_shader;
	// TODO materials
	Material* m_material;

public:
	~Model();
	void CreatePlane(float size);
	void CreateCube(float size);
	void CreateFromOBJ(const std::string& filePath, float size = 1.0, bool faceted = false);
	//void SetShader(Shader* theShader);
	void SetMaterial(Material* theMaterial);
	void draw();
	static void drawAll();
};

