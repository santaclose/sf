#pragma once

#include <vector>
#include <string>

#include <Renderer/Vertex.h>
#include <Renderer/Material.h>

#include <MeshPiece.h>

namespace sf {

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