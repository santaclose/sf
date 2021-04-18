#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Vertex.h>
#include <Object.h>
#include <Material.h>
#include <MeshPiece.h>

namespace sf {

	class MeshReference;
	class MeshProcessor;
	class VoxelModel;

	class Mesh : public Object
	{
		friend MeshReference;
		friend MeshProcessor;
		friend VoxelModel;

		static std::vector<Mesh*> models;

		void SendMatrixToShader(Material& material);

		unsigned int m_gl_vertexBuffer;
		unsigned int m_gl_indexBuffer;
		unsigned int m_gl_vao;
		std::vector<Vertex> m_vertexVector;
		std::vector<unsigned int> m_indexVector;

		std::vector<MeshPiece> m_pieces;

		std::vector<MeshReference*> m_references;

		void CompleteFromVectors();

	public:
		void ReloadVertexData();
		void CreateFromGltf(unsigned int gltfID);
		void CreateFromObj(unsigned int objID);
		void CreateFromCode(void (*generateModelFunc)(), bool smooth = true);
		void CreateFromVoxelModel(const VoxelModel& voxelModel);

		void SetMaterial(Material* theMaterial, int piece);
		void Draw();
		static void DrawAll();
	};
}