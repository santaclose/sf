#include "ModelLoader.h"
#include <fstream>
#include <iostream>

void ModelLoader::LoadOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor)
{
	std::ifstream infile(filePath);
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
//				addIndex(theFace.verts[0]);
//				addIndex(theFace.verts[i - 1]);
//				addIndex(theFace.verts[i]);
			}
		}
	}
	std::cout << "obj loaded\n";
}