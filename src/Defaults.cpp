#include "Defaults.h"
#include <Importer/ObjImporter.h>
#include <MeshData.h>

sf::MeshData sf::Defaults::cubeMeshData;
sf::MeshData sf::Defaults::monkeMeshData;
sf::DataLayout sf::Defaults::defaultVertexLayout;

void sf::Defaults::Initialize()
{	
	defaultVertexLayout = DataLayout({
			{MeshData::VertexAttribute::Position, DataType::vec3f32},
			{MeshData::VertexAttribute::Normal, DataType::vec3f32},
			{MeshData::VertexAttribute::Tangent, DataType::vec3f32},
			{MeshData::VertexAttribute::Bitangent, DataType::vec3f32},
			{MeshData::VertexAttribute::Color, DataType::vec3f32},
			{MeshData::VertexAttribute::UV, DataType::vec2f32},
			{MeshData::VertexAttribute::AO, DataType::f32}
		});

	int objId;
	objId = ObjImporter::Load("assets/meshes/unitCube.obj");
	ObjImporter::GenerateMeshData(objId, cubeMeshData);
	objId = ObjImporter::Load("assets/meshes/monke.obj");
	ObjImporter::GenerateMeshData(objId, monkeMeshData);
}
