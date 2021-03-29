#include "ObjController.h"

#include <regex>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>

namespace ObjController {

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
	struct ObjModel {

		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;
		std::vector<ObjFace> faces;
	};

	std::regex vertexRegex("^v\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s*$");
	std::regex normalsRegex("^vn\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s*$");
	std::regex texCoordsRegex("^vt\\s+(-?\\d*\\.\\d*)\\s+(-?\\d*\\.\\d*)\\s*$");
	std::regex faceRegex("\\d+\\/\\d*\\/\\d*");
	std::regex faceVertexRegex("^(\\d+)\/(\\d*)\/(\\d*)$");

	std::vector<ObjModel> models;


	//struct TempVertex // hold vertices and its duplicates
	//{
	//	int vtxID;
	//	int coordID;
	//};
	//struct TempFace
	//{
	//	std::vector<TempVertex> verts;
	//};

	//bool haveToDuplicateVertex(const TempVertex& a, const std::vector<TempVertex>& vector)
	//{
	//	for (int i = 0; i < vector.size(); i++)
	//	{
	//		if (vector[i].vtxID == a.vtxID && vector[i].coordID != a.coordID)
	//			return true;
	//	}
	//	return false;
	//}
}

int ObjController::Load(const std::string& filePath)
{
	std::ifstream infile(filePath);
	if (infile.fail())
		std::cout << "[ObjLoader] Could not read file: " << filePath << "\n";

	models.emplace_back();
	std::string line;
	while (std::getline(infile, line))
	{
		if (line.find("v ") == 0)
		{
			std::smatch m;
			std::regex_match(line, m, vertexRegex);
			models.back().positions.push_back({ std::stof(m[1].str()) , std::stof(m[2].str()) , std::stof(m[3].str()) });
		}
		else if (line.find("vn ") == 0)
		{
			std::smatch m;
			std::regex_match(line, m, normalsRegex);
			models.back().normals.push_back({ std::stof(m[1].str()) , std::stof(m[2].str()) , std::stof(m[3].str()) });
			glm::normalize(models.back().normals.back());
		}
		else if (line.find("vt ") == 0)
		{
			glm::vec2 newUV;
			std::smatch m;
			std::regex_match(line, m, texCoordsRegex);
			models.back().texCoords.push_back({ std::stof(m[1].str()) , std::stof(m[2].str()) });
		}
		else if (line.find("f ") == 0)
		{
			models.back().faces.emplace_back();
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
				models.back().faces.back().vertices.push_back(nVtx);

				iter++;
			}
		}
	}

	return models.size() - 1;
}

void ObjController::Destroy(int id)
{
}

void ObjController::GetModel(int id, int meshIndex, std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector)
{
	// triangulate
	std::vector<ObjVertex> triangulatedVtxSequence;
	for (ObjFace& f : models[id].faces)
	{
		for (int i = 2; i < f.vertices.size(); i++)
		{
			triangulatedVtxSequence.push_back(f.vertices[0]);
			triangulatedVtxSequence.push_back(f.vertices[i - 1]);
			triangulatedVtxSequence.push_back(f.vertices[i]);
			
			//std::cout << f.vertices[0] << std::endl;
			//std::cout << f.vertices[i - 1] << std::endl;
			//std::cout << f.vertices[i] << std::endl;
			//std::cout << "---" << std::endl;
		}
	}

	// build mesh
	std::unordered_map<ObjVertex, unsigned int, ObjVertexHash> uniqueVertices;
	for (const ObjVertex& v : triangulatedVtxSequence)
	{
		//if (uniqueVertices.find(v) == uniqueVertices.end()) // not found
		{
			vertexVector.emplace_back();
			vertexVector.back().position = models[id].positions[v.posID];
			if (models[id].normals.size() > 0)
				vertexVector.back().normal = models[id].normals[v.normalID];
			if (models[id].texCoords.size() > 0)
				vertexVector.back().textureCoord = models[id].texCoords[v.coordsID];
			uniqueVertices.insert({ v, vertexVector.size() - 1 });
		}
		indexVector.push_back(uniqueVertices[v]);
	}
}
