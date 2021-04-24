#include "ObjImporter.h"

#include <regex>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

namespace sf::ObjImporter {

	struct ObjVertex {

		unsigned int posID;
		unsigned int normalID;
		unsigned int coordsID;
	};
	std::ostream& operator<<(std::ostream& o, const ObjVertex& v)
	{
		o << v.posID << "/" << v.coordsID << "/" << v.normalID;
		return o;
	}

	bool operator==(const ObjVertex& l, const ObjVertex& r)
	{
		return l.posID == r.posID && l.normalID == r.normalID && l.coordsID == r.coordsID;
	}
	struct ObjVertexHash {
		std::size_t operator()(const ObjVertex& v) const
		{
			std::string toHash = std::to_string(v.posID) + '/' + std::to_string(v.normalID) + '/' + std::to_string(v.coordsID);
			return std::hash<std::string>()(toHash);
		}
	};
	struct ObjFace {

		std::vector<ObjVertex> vertices;
	};
	struct ObjMesh {

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;
		std::vector<ObjFace> faces;
		std::unordered_set<unsigned int> pieces;
	};

	std::regex vertexRegex("^v\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s*$");
	std::regex normalsRegex("^vn\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s*$");
	std::regex texCoordsRegex("^vt\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s*$");
	std::regex faceRegex("\\d+\\/\\d*\\/\\d*");
	std::regex faceVertexRegex("^(\\d+)\/(\\d*)\/(\\d*)$");

	std::vector<ObjMesh> meshes;
}

int sf::ObjImporter::Load(const std::string& filePath)
{
	std::ifstream infile(filePath);
	if (infile.fail())
		std::cout << "[ObjImporter] Could not read file: " << filePath << "\n";

	meshes.emplace_back();
	std::string line;
	while (std::getline(infile, line))
	{
		if (line.find("o ") == 0 || line.find("usemtl ") == 0)
		{
			meshes.back().pieces.insert(meshes.back().faces.size());
		}
		else if (line.find("v ") == 0)
		{
			if (meshes.back().pieces.size() == 0)
				meshes.back().pieces.insert(meshes.back().faces.size());

			std::smatch m;
			std::regex_match(line, m, vertexRegex);
			meshes.back().positions.push_back({ std::stof(m[1].str()) , std::stof(m[2].str()) , std::stof(m[3].str()) });
		}
		else if (line.find("vn ") == 0)
		{
			std::smatch m;
			std::regex_match(line, m, normalsRegex);
			meshes.back().normals.push_back({ std::stof(m[1].str()) , std::stof(m[2].str()) , std::stof(m[3].str()) });
			glm::normalize(meshes.back().normals.back());
		}
		else if (line.find("vt ") == 0)
		{
			glm::vec2 newUV;
			std::smatch m;
			std::regex_match(line, m, texCoordsRegex);
			meshes.back().texCoords.push_back({ std::stof(m[1].str()) , std::stof(m[2].str()) });
		}
		else if (line.find("f ") == 0)
		{
			meshes.back().faces.emplace_back();
			std::sregex_iterator iter(line.begin(), line.end(), faceRegex);
			std::sregex_iterator end;

			while (iter != end)
			{
				std::smatch m;
				std::string vertexInfo = (*iter)[0].str();
				std::regex_match(vertexInfo, m, faceVertexRegex);

				ObjVertex nVtx;
				nVtx.posID = std::stoi(m[1].str()) - 1;
				std::string coords = m[2].str();
				std::string normal = m[3].str();
				if (coords.length() > 0)
					nVtx.coordsID = std::stoi(coords) - 1;
				if (coords.length() > 0)
					nVtx.normalID = std::stoi(normal) - 1;
				meshes.back().faces.back().vertices.push_back(nVtx);

				iter++;
			}
		}
	}

	return meshes.size() - 1;
}

void sf::ObjImporter::Destroy(int id)
{
}

void sf::ObjImporter::GetMesh(int id, Mesh& mesh)
{
	assert(id > -1 && id < meshes.size());

	// triangulate
	std::vector<ObjVertex> triangulatedVtxSequence;

	for (int fi = 0; fi < meshes[id].faces.size(); fi++)
	{
		if (meshes[id].pieces.find(fi) != meshes[id].pieces.end())
			mesh.pieces.push_back(triangulatedVtxSequence.size());

		const ObjFace& f = meshes[id].faces[fi];
		for (int i = 2; i < f.vertices.size(); i++)
		{
			triangulatedVtxSequence.push_back(f.vertices[0]);
			triangulatedVtxSequence.push_back(f.vertices[i - 1]);
			triangulatedVtxSequence.push_back(f.vertices[i]);
		}
	}

	// build mesh
	std::unordered_map<ObjVertex, unsigned int, ObjVertexHash> uniqueVertices;

	for (int vi = 0; vi < triangulatedVtxSequence.size(); vi++)
	{
		const ObjVertex& v = triangulatedVtxSequence[vi];
		if (uniqueVertices.find(v) == uniqueVertices.end()) // not found
		{
			mesh.vertexVector.emplace_back();
			mesh.vertexVector.back().position = meshes[id].positions[v.posID];
			if (meshes[id].normals.size() > 0)
				mesh.vertexVector.back().normal = meshes[id].normals[v.normalID];
			if (meshes[id].texCoords.size() > 0)
				mesh.vertexVector.back().textureCoord = meshes[id].texCoords[v.coordsID];
			uniqueVertices.insert({ v, mesh.vertexVector.size() - 1 });
		}
		mesh.indexVector.push_back(uniqueVertices[v]);
	}
}
