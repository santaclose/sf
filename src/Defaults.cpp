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
		objId = ObjImporter::Load("assets/meshes/unitCube.obj");
		ObjImporter::GenerateMeshData(objId, defaultVertexLayout, cubeMeshData);
		return cubeMeshData;
	}

	const MeshData& MeshDataPlane()
	{
		if (planeMeshData.Initialized())
			return planeMeshData;
		objId = ObjImporter::Load("assets/meshes/unitPlane.obj");
		ObjImporter::GenerateMeshData(objId, defaultVertexLayout, planeMeshData);
		return planeMeshData;
	}

	const MeshData& MeshDataSphere()
	{
		if (sphereMeshData.Initialized())
			return sphereMeshData;
		objId = ObjImporter::Load("assets/meshes/unitSphere.obj");
		ObjImporter::GenerateMeshData(objId, defaultVertexLayout, sphereMeshData);
		return sphereMeshData;
	}

	const MeshData& MeshDataMonkey()
	{
		if (monkeyMeshData.Initialized())
			return monkeyMeshData;
		objId = ObjImporter::Load("assets/meshes/monke.obj");
		ObjImporter::GenerateMeshData(objId, defaultVertexLayout, monkeyMeshData);
		return monkeyMeshData;
	}
}