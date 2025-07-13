#include "ObjImporter.h"

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

namespace sf::ObjImporter {

	struct ObjVertex {

		uint32_t posID;
		uint32_t normalID;
		uint32_t coordsID;
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
		std::unordered_set<uint32_t> pieces;
	};

	void parseVec3(const std::string& line, glm::vec3& target, int start = 0)
	{
		int q, p;
		for (q = start; line[q] == ' ' || line[q] == '\t'; q++);
		for (p = q; line[p] != ' ' && line[p] != '\t'; p++);
		target.x = std::stof(line.substr(q, p - q));
		for (q = p; line[q] == ' ' || line[q] == '\t'; q++);
		for (p = q; line[p] != ' ' && line[p] != '\t'; p++);
		target.y = std::stof(line.substr(q, p - q));
		for (q = p; line[q] == ' ' || line[q] == '\t'; q++);
		for (p = q; p < line.length() && line[p] != ' ' && line[p] != '\t'; p++);
		target.z = std::stof(line.substr(q, p - q));
	}
	void parseVec2(const std::string& line, glm::vec2& target, int start = 0)
	{
		int q, p;
		for (q = start; line[q] == ' ' || line[q] == '\t'; q++);
		for (p = q; line[p] != ' ' && line[p] != '\t'; p++);
		target.x = std::stof(line.substr(q, p - q));
		for (q = p; line[q] == ' ' || line[q] == '\t'; q++);
		for (p = q; p < line.length() && line[p] != ' ' && line[p] != '\t'; p++);
		target.y = std::stof(line.substr(q, p - q));
	}
	void parseFace(const std::string& line, std::vector<glm::ivec3>& target, int start = 0)
	{
		int q = start, p = start;
		while (p < line.length())
		{
			for (q = p + 1; line[q] == ' ' || line[q] == '\t'; q++);
			for (p = q; line[p] != '/'; p++);
			int a = p - q == 0 ? -1 : std::stoi(line.substr(q, p - q));
			q = p + 1;
			for (p = q; line[p] != '/'; p++);
			int b = p - q == 0 ? -1 : std::stoi(line.substr(q, p - q));
			q = p + 1;
			for (p = q; p < line.length() && line[p] != ' ' && line[p] != '\t'; p++);
			int c = p - q == 0 ? -1 : std::stoi(line.substr(q, p - q));
			target.push_back({ a, b, c });
			bool moreVerticesAhead = false;
			for (int i = p; i < line.length(); i++)
			{
				if (line[i] >= '0' && line[i] <= '9')
				{
					moreVerticesAhead = true;
					break;
				}
			}
			if (!moreVerticesAhead)
				break;

		}
	}

	std::vector<ObjMesh*> meshes;
}

int sf::ObjImporter::Load(const std::string& filePath)
{
	std::ifstream infile(filePath);
	if (infile.fail())
		std::cout << "[ObjImporter] Could not read file: " << filePath << "\n";

	ObjMesh* newObjMesh = new ObjMesh();

	std::string line;
	while (std::getline(infile, line))
	{
		if (line.find("o ") == 0 || line.find("usemtl ") == 0)
		{
			newObjMesh->pieces.insert(newObjMesh->faces.size());
		}
		else if (line.find("v ") == 0)
		{
			if (newObjMesh->pieces.size() == 0)
				newObjMesh->pieces.insert(newObjMesh->faces.size());

			newObjMesh->positions.emplace_back();
			parseVec3(line, newObjMesh->positions.back(), 1);
		}
		else if (line.find("vn ") == 0)
		{
			newObjMesh->normals.emplace_back();
			parseVec3(line, newObjMesh->normals.back(), 2);

			glm::normalize(newObjMesh->normals.back());
		}
		else if (line.find("vt ") == 0)
		{
			newObjMesh->texCoords.emplace_back();
			parseVec2(line, newObjMesh->texCoords.back(), 2);
		}
		else if (line.find("f ") == 0)
		{
			newObjMesh->faces.emplace_back();

			std::vector<glm::ivec3> parseTarget;
			parseFace(line, parseTarget, 1);
			ObjVertex nVtx;
			for (const glm::ivec3& v : parseTarget)
			{
				ObjVertex nVtx;
				if (v.x > -1) nVtx.posID = v.x - 1;
				if (v.y > -1) nVtx.coordsID = v.y - 1;
				if (v.z > -1) nVtx.normalID = v.z - 1;
				newObjMesh->faces.back().vertices.push_back(nVtx);
			}
		}
	}

	meshes.push_back(newObjMesh);
	return meshes.size() - 1;
}

void sf::ObjImporter::Destroy(int id)
{
	assert(id > -1 && id < meshes.size());
	delete meshes[id];
	meshes[id] = nullptr;
}

void sf::ObjImporter::GenerateMeshData(int id, MeshData& mesh)
{
	DataType positionDataType = mesh.vertexLayout.GetComponent(MeshData::VertexAttribute::Position)->dataType;
	DataType normalDataType = mesh.vertexLayout.GetComponent(MeshData::VertexAttribute::Normal)->dataType;
	DataType uvsDataType = mesh.vertexLayout.GetComponent(MeshData::VertexAttribute::UV)->dataType;

	assert(positionDataType == DataType::vec3f32);
	assert(normalDataType == DataType::vec3f32);
	assert(uvsDataType == DataType::vec2f32);

	assert(id > -1 && id < meshes.size());
	assert(meshes[id] != nullptr);

	// triangulate
	std::vector<ObjVertex> triangulatedVtxSequence;

	for (int fi = 0; fi < meshes[id]->faces.size(); fi++)
	{
		if (meshes[id]->pieces.find(fi) != meshes[id]->pieces.end())
			mesh.pieces.push_back(triangulatedVtxSequence.size());

		const ObjFace& f = meshes[id]->faces[fi];
		for (int i = 2; i < f.vertices.size(); i++)
		{
			triangulatedVtxSequence.push_back(f.vertices[0]);
			triangulatedVtxSequence.push_back(f.vertices[i - 1]);
			triangulatedVtxSequence.push_back(f.vertices[i]);
		}
	}

	// build mesh
	std::unordered_map<ObjVertex, uint32_t, ObjVertexHash> uniqueVertices;
	std::vector<ObjVertex> finalVertices;

	for (int vi = 0; vi < triangulatedVtxSequence.size(); vi++)
	{
		int vertexCounter = 0;
		const ObjVertex& v = triangulatedVtxSequence[vi];
		if (uniqueVertices.find(v) == uniqueVertices.end()) // not found
		{
			finalVertices.push_back(v);
			uniqueVertices.insert({ v, finalVertices.size() - 1 });
		}
		mesh.indexVector.push_back(uniqueVertices[v]);
	}

	if (mesh.vertexBuffer != nullptr)
		free(mesh.vertexBuffer);
	mesh.vertexBuffer = malloc(mesh.vertexLayout.GetSize() * finalVertices.size());
	for (int i = 0; i < finalVertices.size(); i++)
	{
		glm::vec3* posPtr = (glm::vec3*) mesh.vertexLayout.Access(mesh.vertexBuffer, MeshData::VertexAttribute::Position, i);
		*posPtr = meshes[id]->positions[finalVertices[i].posID];
		if (meshes[id]->normals.size() > 0)
		{
			glm::vec3* normalPtr = (glm::vec3*)mesh.vertexLayout.Access(mesh.vertexBuffer, MeshData::VertexAttribute::Normal, i);
			*normalPtr = meshes[id]->normals[finalVertices[i].normalID];
		}
		if (meshes[id]->texCoords.size() > 0)
		{
			glm::vec2* coordsPtr = (glm::vec2*)mesh.vertexLayout.Access(mesh.vertexBuffer, MeshData::VertexAttribute::UV, i);
			*coordsPtr = meshes[id]->texCoords[finalVertices[i].coordsID];
		}
	}
	mesh.vertexCount = finalVertices.size();
}
