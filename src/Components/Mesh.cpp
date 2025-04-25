#include "Mesh.h"

sf::Mesh::Mesh(const MeshData* meshData)
{
	this->meshData = meshData;
	this->materials.resize(meshData->pieces.size(), ~0U);
}

sf::Mesh::Mesh(const MeshData* meshData, uint32_t material)
{
	this->meshData = meshData;
	this->materials.resize(meshData->pieces.size(), material);
}
