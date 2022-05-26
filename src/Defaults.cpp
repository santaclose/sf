#include "Defaults.h"
#include <Importer/ObjImporter.h>

sf::MeshData sf::Defaults::cubeMeshData;
sf::MeshData sf::Defaults::monkeMeshData;

void sf::Defaults::Initialize()
{	
	int objId;
	objId = ObjImporter::Load("assets/meshes/unitCube.obj");
	ObjImporter::GenerateMeshData(objId, cubeMeshData);
	objId = ObjImporter::Load("assets/meshes/monke.obj");
	ObjImporter::GenerateMeshData(objId, monkeMeshData);
}
