#include "Defaults.h"
#include <Importer/ObjImporter.h>

sf::Shader sf::Defaults::shader;
sf::Material sf::Defaults::material;
sf::Mesh sf::Defaults::cubeMesh;

void sf::Defaults::Initialize()
{
	shader.CreateFromFiles("assets/shaders/defaultV.shader", "assets/shaders/defaultF.shader");
	material.CreateFromShader(&shader, true);
	
	int cubeObjId = ObjImporter::Load("assets/unitCube.obj");
	ObjImporter::GetMesh(cubeObjId, cubeMesh);
}
