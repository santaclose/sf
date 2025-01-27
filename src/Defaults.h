#pragma once

#include <MeshData.h>
#include <DataLayout.h>

namespace sf::Defaults
{
	const MeshData& MeshDataCube();
	const MeshData& MeshDataPlane();
	const MeshData& MeshDataSphere();
	const MeshData& MeshDataMonkey();

	DataLayout VertexLayout();
	DataLayout VertexLayoutSkinning();
}