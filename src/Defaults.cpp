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
	BufferLayout defaultVertexLayout = BufferLayout({
		BufferComponent::Position,
		BufferComponent::Normal,
		BufferComponent::UV,
	});

	const MeshData& MeshDataCube()
	{
		if (cubeMeshData.Initialized())
			return cubeMeshData;
		cubeMeshData.vertexBufferLayout = &defaultVertexLayout;
		objId = ObjImporter::Load("assets/meshes/unitCube.obj");
		ObjImporter::GenerateMeshData(objId, cubeMeshData);
		return cubeMeshData;
	}

	const MeshData& MeshDataPlane()
	{
		if (planeMeshData.Initialized())
			return planeMeshData;
		planeMeshData.vertexBufferLayout = &defaultVertexLayout;
		objId = ObjImporter::Load("assets/meshes/unitPlane.obj");
		ObjImporter::GenerateMeshData(objId, planeMeshData);
		return planeMeshData;
	}

	const MeshData& MeshDataSphere()
	{
		if (sphereMeshData.Initialized())
			return sphereMeshData;
		sphereMeshData.vertexBufferLayout = &defaultVertexLayout;
		objId = ObjImporter::Load("assets/meshes/unitSphere.obj");
		ObjImporter::GenerateMeshData(objId, sphereMeshData);
		return sphereMeshData;
	}

	const MeshData& MeshDataMonkey()
	{
		if (monkeyMeshData.Initialized())
			return monkeyMeshData;
		monkeyMeshData.vertexBufferLayout = &defaultVertexLayout;
		objId = ObjImporter::Load("assets/meshes/monke.obj");
		ObjImporter::GenerateMeshData(objId, monkeyMeshData);
		return monkeyMeshData;
	}
}