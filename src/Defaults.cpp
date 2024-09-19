#include "Defaults.h"
#include <Importer/ObjImporter.h>
#include <MeshData.h>

sf::MeshData sf::Defaults::cubeMeshData;
sf::MeshData sf::Defaults::monkeMeshData;
sf::DataLayout sf::Defaults::defaultVertexLayout;
sf::DataLayout sf::Defaults::defaultSkinningVertexLayout;

void sf::Defaults::Initialize()
{	
	int objId;
	objId = ObjImporter::Load("assets/meshes/unitCube.obj");
	ObjImporter::GenerateMeshData(objId, cubeMeshData);
	objId = ObjImporter::Load("assets/meshes/monke.obj");
	ObjImporter::GenerateMeshData(objId, monkeMeshData);

	defaultVertexLayout = DataLayout({
			{VertexAttribute::Position, DataType::vec3f32},
			{VertexAttribute::Normal, DataType::vec3f32},
			{VertexAttribute::Tangent, DataType::vec3f32},
			{VertexAttribute::Bitangent, DataType::vec3f32},
			{VertexAttribute::Color, DataType::vec3f32},
			{VertexAttribute::TexCoords, DataType::vec2f32},
			{VertexAttribute::AmbientOcclusion, DataType::f32}
		});
	defaultSkinningVertexLayout = DataLayout({
			{VertexAttribute::Position, DataType::vec3f32},
			{VertexAttribute::Normal, DataType::vec3f32},
			{VertexAttribute::Tangent, DataType::vec3f32},
			{VertexAttribute::Bitangent, DataType::vec3f32},
			{VertexAttribute::Color, DataType::vec3f32},
			{VertexAttribute::TexCoords, DataType::vec2f32},
			{VertexAttribute::AmbientOcclusion, DataType::f32},
			{VertexAttribute::BoneIndices, DataType::vec4f32},
			{VertexAttribute::BoneWeights, DataType::vec4f32}
		});
}
