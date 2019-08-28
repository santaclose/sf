#include "ModelLoader.h"
#include <fstream>
#include <iostream>

void ModelLoader::LoadOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor)
{
	std::ifstream infile(filePath);
	if (infile.fail()) std::cout << "file " << filePath << " not found\n";
	std::string line;

	glm::vec3 currentVertexPosition;
	while (std::getline(infile, line))
	{
		if (line.find("v ") == 0)
		{
			int p = 2;
			int q = 2;
			for (; line[q] != ' '; q++);
			currentVertexPosition.x = scaleFactor * std::stof(line.substr(p, q - p));
			p = q = q + 1;
			for (; line[q] != ' '; q++);
			currentVertexPosition.y = scaleFactor * std::stof(line.substr(p, q - p));
			p = q = q + 1;
			for (; q < line.size() && line[q] != '\n'; q++);
			currentVertexPosition.z = scaleFactor * std::stof(line.substr(p, q - p));
			vertexVector.emplace_back(currentVertexPosition);
		}
		else if (line.find("f ") == 0)
		{
			std::vector<int> faceVerts;
			faceVerts.reserve(3); // at least 3 vertices for a face
			for (int i = 1; i < line.size(); i++) // from the space character
			{
				if (line[i] == ' ')
				{
					int q = i = i + 1;
					for (; line[q] != '/'; q++);
					faceVerts.push_back(std::stoi(line.substr(i, q - i)));
				}
			}


			for (int i = 2; i < faceVerts.size(); i++) // triangulate
			{
				indexVector.push_back(faceVerts[0] - 1);
				indexVector.push_back(faceVerts[i-1] - 1);
				indexVector.push_back(faceVerts[i] - 1);

				glm::vec3 faceNormal = glm::cross((vertexVector[faceVerts[i-1] - 1].position - vertexVector[faceVerts[0] - 1].position), (vertexVector[faceVerts[i] - 1].position - vertexVector[faceVerts[0] - 1].position));
				vertexVector[faceVerts[0] - 1].normal += faceNormal;
				vertexVector[faceVerts[i - 1] - 1].normal += faceNormal;
				vertexVector[faceVerts[i] - 1].normal += faceNormal;
//				addIndex(theFace.verts[0]);
//				addIndex(theFace.verts[i - 1]);
//				addIndex(theFace.verts[i]);
			}
		}
	}
	//std::cout << "obj loaded\n";
}

void ModelLoader::LoadFacetedOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor)
{
	std::vector<Vertex> tvv; // temporal vertex vector
	std::vector<float> tcv; // temporal coord vector

	std::ifstream infile(filePath);
	if (infile.fail()) std::cout << "file " << filePath << " not found\n";
	std::string line;

	float currentU, currentV;
	glm::vec3 currentVertexPosition;
	while (std::getline(infile, line))
	{
		if (line.find("v ") == 0)
		{
			int p = 2;
			int q = 2;
			for (; line[q] != ' '; q++);
			currentVertexPosition.x = scaleFactor * std::stof(line.substr(p, q - p));
			p = q = q + 1;
			for (; line[q] != ' '; q++);
			currentVertexPosition.y = scaleFactor * std::stof(line.substr(p, q - p));
			p = q = q + 1;
			for (; q < line.size() && line[q] != '\n'; q++);
			currentVertexPosition.z = scaleFactor * std::stof(line.substr(p, q - p));
			tvv.emplace_back(currentVertexPosition);
		}
		else if (line.find("vt ") == 0)
		{
			int p = 3;
			int q = 3;
			for (; line[q] != ' '; q++);
			currentU = std::stof(line.substr(p, q - p));
			p = q = q + 1;
			for (; q < line.size() && line[q] != '\n'; q++);
			currentV = std::stof(line.substr(p, q - p));
			tcv.push_back(currentU);
			tcv.push_back(currentV);
		}
		else if (line.find("f ") == 0)
		{
			std::vector<int> faceCoords;
			std::vector<int> faceVerts;
			faceVerts.reserve(3); // at least 3 vertices for a face
			for (int i = 1; i < line.size(); i++) // from the space character
			{
				if (line[i] == ' ')
				{
					int q = i = i + 1;
					for (; line[q] != '/'; q++);
					faceVerts.push_back(std::stoi(line.substr(i, q - i)));

					if (line[q + 1] != '/') // there are texture coordinates
					{
						i = q + 1;
						for (; line[q] != '/'; q++);
						faceCoords.push_back(std::stoi(line.substr(i, q - i)));
					}
				}
			}

			for (int i = 2; i < faceVerts.size(); i++) // triangulate
			{
				glm::vec3 faceNormal = glm::cross((tvv[faceVerts[i - 1] - 1].position - tvv[faceVerts[0] - 1].position), (tvv[faceVerts[i] - 1].position - tvv[faceVerts[0] - 1].position));

				indexVector.push_back(vertexVector.size());
				vertexVector.push_back(tvv[faceVerts[0] - 1]);
				vertexVector.back().normal = faceNormal;
				vertexVector.back().textureCoord.x = tcv[(faceCoords[0] - 1) * 2];
				vertexVector.back().textureCoord.y = tcv[(faceCoords[0] - 1) * 2 + 1];
				indexVector.push_back(vertexVector.size());
				vertexVector.push_back(tvv[faceVerts[i - 1] - 1]);
				vertexVector.back().normal = faceNormal;
				vertexVector.back().textureCoord.x = tcv[(faceCoords[i - 1] - 1) * 2];
				vertexVector.back().textureCoord.y = tcv[(faceCoords[i - 1] - 1) * 2 + 1];
				indexVector.push_back(vertexVector.size());
				vertexVector.push_back(tvv[faceVerts[i] - 1]);
				vertexVector.back().normal = faceNormal;
				vertexVector.back().textureCoord.x = tcv[(faceCoords[i] - 1) * 2];
				vertexVector.back().textureCoord.y = tcv[(faceCoords[i] - 1) * 2 + 1];


				/*
				indexVector.push_back(faceVerts[0] - 1);
				indexVector.push_back(faceVerts[i - 1] - 1);
				indexVector.push_back(faceVerts[i] - 1);

				glm::vec3 faceNormal = glm::cross((vertexVector[faceVerts[i - 1] - 1].position - vertexVector[faceVerts[0] - 1].position), (vertexVector[faceVerts[i] - 1].position - vertexVector[faceVerts[0] - 1].position));
				vertexVector[faceVerts[0] - 1].normal += faceNormal;
				vertexVector[faceVerts[i - 1] - 1].normal += faceNormal;
				vertexVector[faceVerts[i] - 1].normal += faceNormal;*/
				//				addIndex(theFace.verts[0]);
				//				addIndex(theFace.verts[i - 1]);
				//				addIndex(theFace.verts[i]);
			}
		}
	}
	//std::cout << "obj loaded\n";
}