#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Defaults.h>

sf::MeshPiece::MeshPiece(unsigned int indexStart)
{
	this->indexStart = indexStart;
	this->material = &Defaults::material;
}

int sf::Mesh::counter = 0;

sf::Mesh::Mesh()
{
	id = counter;
	counter++;
}

void sf::Mesh::SetMaterial(Material* theMaterial)
{
	for (MeshPiece& piece : pieces)
		piece.material = theMaterial;
}

void sf::Mesh::SetMaterial(Material* theMaterial, unsigned int piece)
{
	pieces[piece].material = theMaterial;
}