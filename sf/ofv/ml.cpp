#include "ml.h"
#include "../Vertex.h"

namespace ofv
{
	namespace ml
	{
		using namespace ::ofv::vl;

		bool calculateVertexNormals = true;
		unsigned int vertexCounter = 0;
		unsigned int indexCounter = 0;

		std::vector<Vertex>* vertices;
		std::vector<unsigned int>* indexList;

		void Initialize(bool smooth, std::vector<Vertex>* theVertices, std::vector<unsigned int>* theIndexList)
		{
			calculateVertexNormals = smooth;
			vertexCounter = indexCounter = 0;
			vertices = theVertices;
			indexList = theIndexList;
		}

		inline vec calcNormal(faceS& theFace)
		{
			glm::vec3 a = ((vertices[0][theFace.verts[1]].position - vertices[0][theFace.verts[0]].position) *
				(vertices[0][theFace.verts[2]].position - vertices[0][theFace.verts[1]].position));
			vec res(a.x, a.y, a.z);
			return res.Normalized();
		}

		void generateFace(faceS& theFace) // adds the face to the buffer
		{
			vec normal = calcNormal(theFace);

			if (calculateVertexNormals)
			{
				for (unsigned int theVertex : theFace.verts) // update all vertex normals of the face
				{
					vertices[0][theVertex].normal.x += normal.x;
					vertices[0][theVertex].normal.y += normal.y;
					vertices[0][theVertex].normal.z += normal.z;
				}
				for (int i = 2; i < theFace.verts.size(); i++) // triangulate
				{
					indexList->push_back(theFace.verts[0]);
					indexList->push_back(theFace.verts[i-1]);
					indexList->push_back(theFace.verts[i]);
					indexCounter += 3;
				}
			}
			else
			{
				Vertex newVertex(glm::vec3(0,0,0));
				for (int i = 2; i < theFace.verts.size(); i++) // triangulate
				{
					newVertex = vertices[0][theFace.verts[0]];
					newVertex.normal.x = normal.x; newVertex.normal.y = normal.y; newVertex.normal.z = normal.z; // set vertex normals equal to face normals
					
					vertices->push_back(newVertex);
					indexList->push_back(vertices->size() - 1);
					newVertex = vertices[0][theFace.verts[i - 1]];
					newVertex.normal.x = normal.x; newVertex.normal.y = normal.y; newVertex.normal.z = normal.z;
					
					vertices->push_back(newVertex);
					indexList->push_back(vertices->size() - 1);
					newVertex = vertices[0][theFace.verts[i]];
					newVertex.normal.x = normal.x; newVertex.normal.y = normal.y; newVertex.normal.z = normal.z;
					
					vertices->push_back(newVertex);
					indexList->push_back(vertices->size() - 1);
					vertexCounter += 3;
				}
			}
		}

		unsigned int vertex(float x, float y, float z, float u, float v)
		{
			Vertex newVertex = Vertex(x, y, z);
			newVertex.textureCoord.x = u;
			newVertex.textureCoord.y = v;
			
			vertices->push_back(newVertex);
			vertexCounter++;
			return vertexCounter - 1;
		}
		unsigned int vertex(vec& pos, float u, float v)
		{
			Vertex newVertex = Vertex(pos.x, pos.y, pos.z);
			newVertex.textureCoord.x = u;
			newVertex.textureCoord.y = v;
			
			vertices->push_back(newVertex);
			vertexCounter++;
			return vertexCounter - 1;
		}
		unsigned int vertex(vec&& pos, float u, float v)
		{
			Vertex newVertex = Vertex(pos.x, pos.y, pos.z);
			newVertex.textureCoord.x = u;
			newVertex.textureCoord.y = v;
			
			vertices->push_back(newVertex);
			vertexCounter++;
			return vertexCounter - 1;
		}

		void face(unsigned int* ids, int length)
		{
			faceS fce;
			for (int i = 0; i < length; i++)
			{
				fce.verts.push_back(ids[i]);
			}
			generateFace(fce);
		}
		void face(unsigned int* ids, int length, bool invert)
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
		void face(unsigned int* ids, int length, int start)
		{
			faceS fce;
			for (int i = 0; i < length; i++)
			{
				fce.verts.push_back(ids[i + start]);
			}
			generateFace(fce);
		}
		void face(unsigned int* ids, int length, int start, bool invert)
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
		void faceSeq(unsigned int* ids, int count, int vertsPerFace)
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