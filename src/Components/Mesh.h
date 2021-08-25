#pragma once

#include <vector>
#include <string>

#include <Renderer/Vertex.h>
#include <Renderer/Material.h>

namespace sf {

	struct MeshPiece
	{
		unsigned int indexStart;
		Material* material = nullptr;
		MeshPiece::MeshPiece(unsigned int indexStart);
	};

	struct Mesh
	{		
		static int counter;

		int id;
		bool vertexReloadPending = true;
		std::vector<Vertex> vertexVector;
		std::vector<unsigned int> indexVector;
		std::vector<MeshPiece> pieces;

		Mesh();

		void SetMaterial(Material* theMaterial);
		void SetMaterial(Material* theMaterial, unsigned int piece);
	};
}