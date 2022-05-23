#include "Mesh.h"

int sf::Mesh::counter = 0;

sf::Mesh::Mesh(const MeshData* meshData)
{
	this->meshData = meshData;
	this->id = counter;
	counter++;
}
