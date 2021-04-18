#include "Mesh.h"

#include <ml.h>
#include <iostream>

#include <MeshReference.h>
#include <Camera.h>
#include <Texture.h>
#include <Random.h>
#include <Importer/GltfImporter.h>
#include <Importer/ObjImporter.h>
#include <VoxelModel.h>

std::vector<sf::Mesh*> sf::Mesh::models;

void sf::Mesh::SendMatrixToShader(Material& material)
{
	if (m_matrixUpdatePending)
		UpdateTransformMatrix();
	material.m_shader->SetUniformMatrix4fv("modelMatrix", &m_transformMatrix[0][0]);
}
void sf::Mesh::CompleteFromVectors()
{
	glGenVertexArrays(1, &m_gl_vao);
	glGenBuffers(1, &m_gl_vertexBuffer);
	glGenBuffers(1, &m_gl_indexBuffer);

	glBindVertexArray(m_gl_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);

	// update vertices
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	// update indices to draw
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(1); // normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(2); // tangent
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(3); // bitangent
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9));
	glEnableVertexAttribArray(4); // texture coords
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 12));
	glEnableVertexAttribArray(5); // extra data
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 14));

	glBindVertexArray(0);

	models.push_back(this);
}
void sf::Mesh::ReloadVertexData()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexVector.size() * sizeof(Vertex), &m_vertexVector[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexVector.size() * sizeof(unsigned int), &m_indexVector[0], GL_STATIC_DRAW);
}

void sf::Mesh::CreateFromGltf(unsigned int gltfID)
{
	GltfImporter::GetMesh(gltfID, m_vertexVector, m_indexVector, m_pieces);
	CompleteFromVectors();
}
void sf::Mesh::CreateFromObj(unsigned int objID)
{
	ObjImporter::GetMesh(objID, m_vertexVector, m_indexVector, m_pieces);
	CompleteFromVectors();
}
void sf::Mesh::CreateFromCode(void (*generateModelFunc)(), bool smooth)
{
	sfmg::ml::Initialize(smooth, &m_vertexVector, &m_indexVector);
	generateModelFunc();
	CompleteFromVectors();
	m_pieces.emplace_back(0);
}

void sf::Mesh::CreateFromVoxelModel(const VoxelModel& voxelModel)
{
	int unitcubeid = ObjImporter::Load("assets/unitcube.obj");
	this->CreateFromObj(unitcubeid);

	for (auto& v : m_vertexVector)
		v.position *= voxelModel.m_voxelSize;

	std::vector<Vertex> cubeV = m_vertexVector;
	std::vector<unsigned int> cubeI = m_indexVector;

	m_vertexVector.clear();
	m_indexVector.clear();

	for (int i = 0; i < voxelModel.m_mat.size(); i++)
	{
		for (int j = 0; j < voxelModel.m_mat[i].size(); j++)
		{
			for (int k = 0; k < voxelModel.m_mat[i][j].size(); k++)
			{
				if (voxelModel.m_mat[i][j][k])
				{
					for (const Vertex& v : cubeV)
					{
						m_vertexVector.emplace_back(v);
						m_vertexVector.back().position += voxelModel.m_minPos;
						m_vertexVector.back().position += glm::vec3(
							i * voxelModel.m_voxelSize + voxelModel.m_voxelSize / 2.0f,
							j * voxelModel.m_voxelSize + voxelModel.m_voxelSize / 2.0f,
							k * voxelModel.m_voxelSize + voxelModel.m_voxelSize / 2.0f);
					}
					unsigned int indexOffset = m_vertexVector.size();
					for (unsigned int index : cubeI)
						m_indexVector.push_back(index + indexOffset);
				}
			}
		}
	}

	ReloadVertexData();
}

void sf::Mesh::SetMaterial(Material* theMaterial, int piece)
{
	assert(m_pieces.size() > piece && piece > -1);
	m_pieces[piece].material = theMaterial;
}
void sf::Mesh::Draw()
{
	for (unsigned int i = 0; i < m_pieces.size(); i++)
	{
		const MeshPiece& mp = m_pieces[i];
		mp.material->Bind();

		mp.material->m_shader->SetUniformMatrix4fv("cameraMatrix", &(Camera::GetMatrix()[0][0]));
		mp.material->m_shader->SetUniform3fv("camPos", &(Camera::boundCamera->GetPosition()[0]));

		SendMatrixToShader(*mp.material);
		
		unsigned int end, start;
		start = m_pieces[i].indexStart;
		end = m_pieces.size() > i + 1 ? m_pieces[i + 1].indexStart : m_indexVector.size();

		glBindVertexArray(m_gl_vao);

		glDrawElements(GL_TRIANGLES, end - start , GL_UNSIGNED_INT, (void*)(start * sizeof(unsigned int)));
		 
		for (MeshReference* m : m_references)
		{
			// replace model matrix and draw again
			m->SendMatrixToShader(*mp.material);
			glDrawElements(GL_TRIANGLES, end - start, GL_UNSIGNED_INT, (void*)(start * sizeof(unsigned int)));
		}
	}

	//m_material->Bind();

	//m_material->m_shader->SetUniformMatrix4fv("cameraMatrix", &(Camera::GetMatrix()[0][0]));
	//m_material->m_shader->SetUniform3fv("camPos", &(Camera::boundCamera->GetPosition()[0]));

	//SendMatrixToShader();
	//
	//unsigned int pieceToRender = 0;
	//unsigned int end, start;
	//start = m_pieces[pieceToRender].indexStart;
	//end = m_pieces.size() > pieceToRender + 1 ? m_pieces[pieceToRender + 1].indexStart : m_indexVector.size();

	//glBindVertexArray(m_gl_vao);

	//glDrawElements(GL_TRIANGLES, end - start , GL_UNSIGNED_INT, (void*)(start * sizeof(unsigned int)));
	// 
	//for (MeshReference* m : m_references)
	//{
	//	// replace model matrix and draw again
	//	m->SendMatrixToShader();
	//	glDrawElements(GL_TRIANGLES, end - start, GL_UNSIGNED_INT, (void*)(start * sizeof(unsigned int)));
	//}
	

	//glBindVertexArray(0);
}
void sf::Mesh::DrawAll()
{
	for (Mesh* m : models)
	{
		m->Draw();
	}
}