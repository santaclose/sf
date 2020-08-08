#include "ModelLoader.h"
#include <fstream>
#include <iostream>

#include "vendor/tiny_gltf/tiny_gltf.h"

struct TempVertex // hold vertices and its duplicates
{
	int vtxID;
	int coordID;
};

inline bool haveToDuplicateVertex(const TempVertex& a, const std::vector<TempVertex>& vector)
{
	for (int i = 0; i < vector.size(); i++)
	{
		if (vector[i].vtxID == a.vtxID && vector[i].coordID != a.coordID)
			return true;
	}
	return false;
}

void ModelLoader::LoadOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor)
{
	std::ifstream infile(filePath);
	if (infile.fail())
		std::cout << "Could not read model file: " << filePath << "\n";

	std::string line;


	struct TempFace
	{
		std::vector<TempVertex> verts;
	};

	std::vector<glm::vec2> coordList;
	std::vector<TempFace> faceList;

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
		else if (line.find("vt ") == 0)
		{
			glm::vec2 newUV;
			int p = 3;
			int q = 3;
			for (; line[q] != ' '; q++);
			newUV.x = std::stof(line.substr(p, q - p));
			p = q = q + 1;
			for (; q < line.size() && line[q] != '\n'; q++);
			newUV.y = std::stof(line.substr(p, q - p));
			coordList.push_back(newUV);
		}
		else if (line.find("f ") == 0)
		{
			TempFace newPolygon;
			//std::vector<int> faceVerts;
			//faceVerts.reserve(3); // at least 3 vertices for a face
			for (int i = 1; i < line.size(); i++) // from the space character
			{
				if (line[i] == ' ')
				{
					TempVertex nVtx;
					int q = i = i + 1;
					for (; line[q] != '/'; q++);
					nVtx.vtxID = std::stoi(line.substr(i, q - i));
					i = q = q + 1;
					for (; line[q] != '/'; q++);
					nVtx.coordID = std::stoi(line.substr(i, q - i));
					newPolygon.verts.push_back(nVtx);
				}
			}

			for (int i = 2; i < newPolygon.verts.size(); i++) // triangulate
			{
				// indexing has to be done at the end cause of the duplicate vertices
				//indexVector.push_back(newFace.verts[0].vtxID - 1);
				//indexVector.push_back(newFace.verts[i-1].vtxID - 1);
				//indexVector.push_back(newFace.verts[i].vtxID - 1);

				glm::vec3 faceNormal = glm::normalize(glm::cross((vertexVector[newPolygon.verts[i - 1].vtxID - 1].position - vertexVector[newPolygon.verts[0].vtxID - 1].position), (vertexVector[newPolygon.verts[i].vtxID - 1].position - vertexVector[newPolygon.verts[0].vtxID - 1].position)));
				
				// tangent space
				glm::vec3 edge1 = vertexVector[newPolygon.verts[i - 1].vtxID - 1].position - vertexVector[newPolygon.verts[0].vtxID - 1].position;
				glm::vec3 edge2 = vertexVector[newPolygon.verts[i].vtxID - 1].position - vertexVector[newPolygon.verts[0].vtxID - 1].position;
				glm::vec2 deltaUV1 = coordList[newPolygon.verts[i - 1].coordID - 1] - coordList[newPolygon.verts[0].coordID - 1];
				glm::vec2 deltaUV2 = coordList[newPolygon.verts[i].coordID - 1] - coordList[newPolygon.verts[0].coordID - 1];

				float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

				glm::vec3 t, b;

				t.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
				t.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
				t.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
				t = glm::normalize(t);

				b.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
				b.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
				b.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
				b = glm::normalize(b);

				//vertexVector[faceVerts[0] - 1].normal += faceNormal;
				//vertexVector[faceVerts[i - 1] - 1].normal += faceNormal;
				//vertexVector[faceVerts[i] - 1].normal += faceNormal;
				Vertex* a = &(vertexVector[newPolygon.verts[0].vtxID - 1]);
				Vertex* be = &(vertexVector[newPolygon.verts[i - 1].vtxID - 1]);
				Vertex* c = &(vertexVector[newPolygon.verts[i].vtxID - 1]);
				a->normal += faceNormal; a->tangent += t; a->bitangent += b;
				be->normal += faceNormal; be->tangent += t; be->bitangent += b;
				c->normal += faceNormal; c->tangent += t; c->bitangent += b;

				TempFace newFace;
				newFace.verts.push_back(newPolygon.verts[0]);
				newFace.verts.push_back(newPolygon.verts[i-1]);
				newFace.verts.push_back(newPolygon.verts[i]);
				faceList.push_back(newFace);
			}
		}
	}

	std::vector<TempVertex> createdVertices;
	for (int i = 0; i < faceList.size(); i++)
	{
		for (int j = 0; j < faceList[i].verts.size(); j++)
		{
			if (haveToDuplicateVertex(faceList[i].verts[j], createdVertices))
			{
				vertexVector.push_back(vertexVector[faceList[i].verts[j].vtxID - 1]);
				vertexVector.back().textureCoord = coordList[faceList[i].verts[j].coordID - 1];
				indexVector.push_back(vertexVector.size() - 1);
			}
			else
			{
				vertexVector[faceList[i].verts[j].vtxID - 1].textureCoord = coordList[faceList[i].verts[j].coordID - 1];
				indexVector.push_back(faceList[i].verts[j].vtxID - 1);
			}
			createdVertices.push_back(faceList[i].verts[j]);
		}
	}

	std::cout << "finished loading obj\n\tvertex count: " << vertexVector.size() << std::endl;
}

void ModelLoader::LoadFacetedOBJFile(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath, float scaleFactor)
{
	std::vector<Vertex> tvv; // temporal vertex vector
	std::vector<float> tcv; // temporal coord vector

	std::ifstream infile(filePath);
	if (infile.fail())
		std::cout << "Could not read model file: " << filePath << "\n";
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
				glm::vec3 faceNormal = glm::normalize( glm::cross((tvv[faceVerts[i - 1] - 1].position - tvv[faceVerts[0] - 1].position), (tvv[faceVerts[i] - 1].position - tvv[faceVerts[0] - 1].position)));

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

				// tangent space
				glm::vec3 edge1 = vertexVector[vertexVector.size() - 2].position - vertexVector[vertexVector.size() - 3].position;
				glm::vec3 edge2 = vertexVector[vertexVector.size() - 1].position - vertexVector[vertexVector.size() - 3].position;
				glm::vec2 deltaUV1 = vertexVector[vertexVector.size() - 2].textureCoord - vertexVector[vertexVector.size() - 3].textureCoord;
				glm::vec2 deltaUV2 = vertexVector[vertexVector.size() - 1].textureCoord - vertexVector[vertexVector.size() - 3].textureCoord;

				float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

				glm::vec3 t, b;

				t.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
				t.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
				t.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
				t = glm::normalize(t);

				b.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
				b.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
				b.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
				b = glm::normalize(b);

				vertexVector.back().tangent = vertexVector[vertexVector.size() - 2].tangent = vertexVector[vertexVector.size() - 3].tangent = t;
				vertexVector.back().bitangent = vertexVector[vertexVector.size() - 2].bitangent = vertexVector[vertexVector.size() - 3].bitangent = b;
			}
		}
	}

	std::cout << "finished loading obj\n\tvertex count: " << vertexVector.size() << std::endl;
}

void ComputeTangentSpace(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector)
{
	for (int i = 0; i < indexVector.size(); i+=3)
	{
		// tangent space
		glm::vec3 edge1 = vertexVector[indexVector[i + 1]].position - vertexVector[indexVector[i]].position;
		glm::vec3 edge2 = vertexVector[indexVector[i + 2]].position - vertexVector[indexVector[i]].position;
		glm::vec2 deltaUV1 = vertexVector[indexVector[i + 1]].textureCoord - vertexVector[indexVector[i]].textureCoord;
		glm::vec2 deltaUV2 = vertexVector[indexVector[i + 2]].textureCoord - vertexVector[indexVector[i]].textureCoord;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		glm::vec3 t, b;

		t.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		t.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		t.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		t = glm::normalize(t);

		b.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		b.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		b.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		b = glm::normalize(b);

		vertexVector[indexVector[i]].tangent += t;
		vertexVector[indexVector[i + 1]].tangent += t;
		vertexVector[indexVector[i + 2]].tangent += t;

		vertexVector[indexVector[i]].bitangent += b;
		vertexVector[indexVector[i + 1]].bitangent += b;
		vertexVector[indexVector[i + 2]].bitangent += b;
	}
}

void ModelLoader::LoadGLTF(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, const std::string& filePath)
{
	using namespace tinygltf;

	tinygltf::Model model;
	TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
	//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

	if (!warn.empty())
		printf("Warn: %s\n", warn.c_str());

	if (!err.empty())
		printf("Err: %s\n", err.c_str());

	if (!ret)
		printf("Failed to parse glTF\n");

	std::vector<glm::vec3> vertex_positions;
	std::vector<glm::vec3> vertex_normals;
	std::vector<glm::vec2> vertex_uv;

	for (auto& x : model.meshes)
	{
		for (auto& prim : x.primitives)
		{
			// Fill indices:
			const tinygltf::Accessor& accessor = model.accessors[prim.indices];
			const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
			int stride = accessor.ByteStride(bufferView);
			size_t indexCount = accessor.count;
			size_t indexOffset = indexVector.size();
			indexVector.resize(indexOffset + indexCount);

			const unsigned char* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;
			int index_remap[3];
			index_remap[0] = 0;
			index_remap[1] = 1;
			index_remap[2] = 2;

			if (stride == 1)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					indexVector[indexOffset + i + 0] = data[i + index_remap[0]];
					indexVector[indexOffset + i + 1] = data[i + index_remap[1]];
					indexVector[indexOffset + i + 2] = data[i + index_remap[2]];
				}
			}
			else if (stride == 2)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					indexVector[indexOffset + i + 0] = ((uint16_t*)data)[i + index_remap[0]];
					indexVector[indexOffset + i + 1] = ((uint16_t*)data)[i + index_remap[1]];
					indexVector[indexOffset + i + 2] = ((uint16_t*)data)[i + index_remap[2]];
				}
			}
			else if (stride == 4)
			{
				for (size_t i = 0; i < indexCount; i += 3)
				{
					indexVector[indexOffset + i + 0] = ((uint32_t*)data)[i + index_remap[0]];
					indexVector[indexOffset + i + 1] = ((uint32_t*)data)[i + index_remap[1]];
					indexVector[indexOffset + i + 2] = ((uint32_t*)data)[i + index_remap[2]];
				}
			}
			else
			{
				assert(0 && "unsupported index stride!");
			}

			for (auto& attr : prim.attributes)
			{
				const std::string& attr_name = attr.first;
				int attr_data = attr.second;

				const tinygltf::Accessor& accessor = model.accessors[attr_data];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				int stride = accessor.ByteStride(bufferView);
				size_t vertexCount = accessor.count;

				const unsigned char* data = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

				if (!attr_name.compare("POSITION"))
				{
					vertex_positions.resize(vertexCount);
					assert(stride == 12);
					for (size_t i = 0; i < vertexCount; ++i)
					{
						vertex_positions[i] = ((glm::vec3*)data)[i];
					}
				}
				else if (!attr_name.compare("NORMAL"))
				{
					vertex_normals.resize(vertexCount);
					assert(stride == 12);
					for (size_t i = 0; i < vertexCount; ++i)
					{
						vertex_normals[i] = ((glm::vec3*)data)[i];
					}
				}
				else if (!attr_name.compare("TEXCOORD_0"))
				{
					vertex_uv.resize(vertexCount);
					assert(stride == 8);
					for (size_t i = 0; i < vertexCount; ++i)
					{
						const glm::vec2& tex = ((glm::vec2*)data)[i];

						vertex_uv[i].x = tex.x;
						vertex_uv[i].y = 1.0-tex.y;
					}
				}
			}
		}
	}

	vertexVector.resize(vertex_positions.size());
	for (int i = 0; i < vertex_positions.size(); i++)
	{
		//std::cout << vertex_uv[i].x << ", " << vertex_uv[i].y << ", " << vertex_uv[i].z << std::endl;
		vertexVector[i].position = vertex_positions[i];
		vertexVector[i].normal = vertex_normals[i];
		vertexVector[i].textureCoord = vertex_uv[i];
	}
	ComputeTangentSpace(vertexVector, indexVector);
}