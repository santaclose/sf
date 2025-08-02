#include "Defaults.h"
#include <Importer/ObjImporter.h>
#include <MeshData.h>

namespace sf::Defaults
{
	int objId;
	MeshData cubeMeshData;
	MeshData planeMeshData;
	MeshData sphereMeshData;
	MeshData monkeyMeshData;

	const MeshData& MeshDataCube()
	{
		if (cubeMeshData.Initialized())
			return cubeMeshData;
		cubeMeshData.vertexBufferLayout = BufferLayout({
			BufferComponent::VertexPosition,
			BufferComponent::VertexNormal,
			BufferComponent::VertexUV,
		});
		objId = ObjImporter::Load("assets/meshes/unitCube.obj");
		ObjImporter::GenerateMeshData(objId, cubeMeshData);
		return cubeMeshData;
	}

	const MeshData& MeshDataPlane()
	{
		if (planeMeshData.Initialized())
			return planeMeshData;
		planeMeshData.vertexBufferLayout = BufferLayout({
			BufferComponent::VertexPosition,
			BufferComponent::VertexNormal,
			BufferComponent::VertexUV,
		});
		objId = ObjImporter::Load("assets/meshes/unitPlane.obj");
		ObjImporter::GenerateMeshData(objId, planeMeshData);
		return planeMeshData;
	}

	const MeshData& MeshDataSphere()
	{
		if (sphereMeshData.Initialized())
			return sphereMeshData;
		sphereMeshData.vertexBufferLayout = BufferLayout({
			BufferComponent::VertexPosition,
			BufferComponent::VertexNormal,
			BufferComponent::VertexUV,
		});
		objId = ObjImporter::Load("assets/meshes/unitSphere.obj");
		ObjImporter::GenerateMeshData(objId, sphereMeshData);
		return sphereMeshData;
	}

	const MeshData& MeshDataMonkey()
	{
		if (monkeyMeshData.Initialized())
			return monkeyMeshData;
		monkeyMeshData.vertexBufferLayout = BufferLayout({
			BufferComponent::VertexPosition,
			BufferComponent::VertexNormal,
			BufferComponent::VertexUV,
		});
		objId = ObjImporter::Load("assets/meshes/monke.obj");
		ObjImporter::GenerateMeshData(objId, monkeyMeshData);
		return monkeyMeshData;
	}
}