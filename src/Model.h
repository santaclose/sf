#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Vertex.h>
#include <Object.h>
#include <Material.h>

namespace sf {

	class ModelReference;
	class ModelProcessor;
	class VoxelModel;

	class Model : public Object
	{
		friend ModelReference;
		friend ModelProcessor;
		friend VoxelModel;

		static std::vector<Model*> models;

		void SendMatrixToShader();

		unsigned int m_gl_vertexBuffer;
		unsigned int m_gl_indexBuffer;
		unsigned int m_gl_vao;
		std::vector<Vertex> m_vertexVector;
		std::vector<unsigned int> m_indexVector;

		Material* m_material;

		std::vector<ModelReference*> m_references;

		void CompleteFromVectors();

	public:
		void ReloadVertexData();
		void CreateFromGltf(unsigned int gltfID, unsigned int meshIndex = 0);
		void CreateFromObj(unsigned int objID, unsigned int meshIndex = 0);
		void CreateFromCode(void (*generateModelFunc)(), bool smooth = true);
		void CreateFromVoxelModel(const VoxelModel& voxelModel);

		void SetMaterial(Material* theMaterial);
		void Draw();
		static void DrawAll();
	};
}