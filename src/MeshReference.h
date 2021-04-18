#pragma once

#include <Object.h>
#include <Material.h>
#include <Mesh.h>

namespace sf {

	// a model reference only stores transform data
	class MeshReference : public Object
	{
		friend Mesh;

		Mesh* m_originalModel;
		void SendMatrixToShader(Material& material);

	public:
		void CreateFomMesh(Mesh& theMesh);
	};
}