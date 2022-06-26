#include "ml.h"

namespace sfmg
{
	namespace ml
	{
		using namespace vl;

		bool calculateVertexNormals = true;
		uint32_t vertexCounter = 0;
		uint32_t indexCounter = 0;

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indexList;

		void Initialize(bool smooth)
		{
			calculateVertexNormals = smooth;
			vertexCounter = indexCounter = 0;
			vertices.clear();
			indexList.clear();
		}

		void GetData(Vertex*& vertexBuffer, uint32_t& vertexCount, const uint32_t*& indexBuffer, uint32_t& indexCount)
		{
			vertexBuffer = &(vertices[0]);
			vertexCount = vertexCounter;
			indexBuffer = &(indexList[0]);
			indexCount = indexCounter;
		}

		inline vec calcNormal(faceS& theFace)
		{
			glm::vec3 a = ((vertices[theFace.verts[1]].position - vertices[theFace.verts[0]].position) *
				(vertices[theFace.verts[2]].position - vertices[theFace.verts[1]].position));
			vec res(a.x, a.y, a.z);
			return res.Normalized();
		}

		void generateFace(faceS& theFace) // adds the face to the buffer
		{
			vec normal = calcNormal(theFace);

			if (calculateVertexNormals)
			{
				for (uint32_t theVertex : theFace.verts) // update all vertex normals of the face
				{
					vertices[theVertex].normal.x += normal.x;
					vertices[theVertex].normal.y += normal.y;
					vertices[theVertex].normal.z += normal.z;
				}
				for (int i = 2; i < theFace.verts.size(); i++) // triangulate
				{
					indexList.push_back(theFace.verts[0]);
					indexList.push_back(theFace.verts[i-1]);
					indexList.push_back(theFace.verts[i]);
					indexCounter += 3;
				}
			}
			else
			{
				Vertex newVertex;
				for (int i = 2; i < theFace.verts.size(); i++) // triangulate
				{
					newVertex = vertices[theFace.verts[0]];
					newVertex.normal.x = normal.x; newVertex.normal.y = normal.y; newVertex.normal.z = normal.z; // set vertex normals equal to face normals
					
					vertices.push_back(newVertex);
					indexList.push_back(vertices.size() - 1);
					newVertex = vertices[theFace.verts[i - 1]];
					newVertex.normal.x = normal.x; newVertex.normal.y = normal.y; newVertex.normal.z = normal.z;
					
					vertices.push_back(newVertex);
					indexList.push_back(vertices.size() - 1);
					newVertex = vertices[theFace.verts[i]];
					newVertex.normal.x = normal.x; newVertex.normal.y = normal.y; newVertex.normal.z = normal.z;
					
					vertices.push_back(newVertex);
					indexList.push_back(vertices.size() - 1);
					vertexCounter += 3;
				}
			}
		}

		uint32_t vertex(float x, float y, float z, float u, float v)
		{
			Vertex newVertex;
			newVertex.position = { x, y, z };
			newVertex.textureCoord = { u, v };
			
			vertices.push_back(newVertex);
			vertexCounter++;
			return vertexCounter - 1;
		}
		uint32_t vertex(const vec& pos, float u, float v)
		{
			Vertex newVertex;
			newVertex.position = { pos.x, pos.y, pos.z };
			newVertex.textureCoord = { u, v };
			
			vertices.push_back(newVertex);
			vertexCounter++;
			return vertexCounter - 1;
		}

		void face(uint32_t* ids, int length)
		{
			faceS fce;
			for (int i = 0; i < length; i++)
			{
				fce.verts.push_back(ids[i]);
			}
			generateFace(fce);
		}
		void face(uint32_t* ids, int length, bool invert)
		{
			faceS fce;
			if (invert)
			{
				for (int i = length - 1; i > -1; i--)
				{
					fce.verts.push_back(ids[i]);
				}
				generateFace(fce);
			}
			else
			{
				for (int i = 0; i < length; i++)
				{
					fce.verts.push_back(ids[i]);
				}
				generateFace(fce);
			}
		}
		void face(uint32_t* ids, int length, int start)
		{
			faceS fce;
			for (int i = 0; i < length; i++)
			{
				fce.verts.push_back(ids[i + start]);
			}
			generateFace(fce);
		}
		void face(uint32_t* ids, int length, int start, bool invert)
		{
			faceS fce;
			if (invert)
			{
				for (int i = length - 1; i > -1; i--)
				{
					fce.verts.push_back(ids[i + start]);
				}
			}
			else
			{
				for (int i = 0; i < length; i++)
				{
					fce.verts.push_back(ids[i + start]);
				}
			}
			generateFace(fce);
		}
		void faceSeq(uint32_t* ids, int count, int vertsPerFace)
		{
			int length = count / vertsPerFace;
			faceS* fce = (faceS*)alloca(length * sizeof(faceS));

			int curFce = 0;

			for (int i = 0; i < count; i++)
			{
				fce[curFce].verts.push_back(ids[i]);
				if ((i + 1) % vertsPerFace == 0)
				{
					generateFace(fce[curFce]);
					curFce++;
				}
			}
		}
	}
}