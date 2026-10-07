#pragma once

#include <vector>
#include <string>

#include <MeshData.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace sf::ObjImporter
{
	struct ObjMarkerExt {

		std::string name;
		glm::vec3 position;
		glm::quat rotation;
	};

	int Load(const std::string& filePath);
	void Destroy(int id);
	bool GenerateMeshData(int id, Buffer::Layout vertexLayout, MeshData& meshData);
	const std::vector<ObjMarkerExt>* GetMarkers(int id);
	void FreeMeshData(MeshData& mesh);
}