#pragma once

#include <glm/glm.hpp>
#include <MeshData.h>

#define ERRT_RADIUS 0.05
namespace errt
{
	int seed = 0;
	int maxCount = 40;

	std::vector<glm::vec3> vertices;
	std::vector<uint32_t> indices;

	inline uint32_t PushVertex(const glm::vec3& vertex)
	{
		vertices.push_back(vertex);
		return vertices.size() - 1;
	}

	inline void PushFace(uint32_t* pointer, uint32_t vertexCount, bool invert = false)
	{
		if (invert)
			for (uint32_t i = 2; i < vertexCount; i++)
			{
				indices.push_back(pointer[i]);
				indices.push_back(pointer[i - 1]);
				indices.push_back(pointer[0]);
			}
		else
			for (uint32_t i = 2; i < vertexCount; i++)
			{
				indices.push_back(pointer[0]);
				indices.push_back(pointer[i - 1]);
				indices.push_back(pointer[i]);
			}
	}

	void Cylinder(float radius, int hSteps, int vSteps, const glm::vec3& posA, const glm::vec3& posB, bool cap = true)
	{
		glm::vec3 dir = (posB - posA);
		float distance = glm::length(dir);
		dir /= distance;
		float test = glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), dir);
		glm::vec3 localRight = glm::abs(test) == 1.0 ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 localForward = glm::normalize(glm::cross(dir, localRight));

		const float PI = 3.14159265358;

		float angleStep = 2.0 * PI / (double)hSteps;
		uint32_t quad[4];
		uint32_t* vi = (uint32_t*)alloca(sizeof(uint32_t) * hSteps * 2);

		uint32_t* capA = (uint32_t*)alloca(sizeof(uint32_t) * hSteps);
		uint32_t* capB = (uint32_t*)alloca(sizeof(uint32_t) * hSteps);

		for (int j = 0; j < vSteps; j++)
		{
			glm::vec3 currentA = posA + ((float)(j + 0) * distance / (float)vSteps) * dir;
			glm::vec3 currentB = posA + ((float)(j + 1) * distance / (float)vSteps) * dir;
			for (int i = 0; i < hSteps; i++)
			{
				if (j == 0)
					vi[i * 2 + 0] = PushVertex(currentA + localRight * glm::cos(angleStep * i) * radius + localForward * glm::sin(angleStep * i) * radius);
				else
					vi[i * 2 + 0] = vi[i * 2 + 1];
				vi[i * 2 + 1] = PushVertex(currentB + localRight * glm::cos(angleStep * i) * radius + localForward * glm::sin(angleStep * i) * radius);
				if (j == 0)
					capA[i] = vi[i * 2 + 0];
				if (j == vSteps - 1)
					capB[i] = vi[i * 2 + 1];

				if (i > 0)
				{
					quad[0] = vi[i * 2 - 2];
					quad[1] = vi[i * 2];
					quad[2] = vi[i * 2 + 1];
					quad[3] = vi[i * 2 - 1];
					PushFace(quad, 4);
				}
			}
			quad[0] = quad[1];
			quad[3] = quad[2];
			quad[1] = vi[0];
			quad[2] = vi[1];
			PushFace(quad, 4);
		}

		if (!cap)
			return;

		PushFace(capA, hSteps, true);
		PushFace(capB, hSteps, false);
	}

	void GenerateModel(sf::MeshData& mesh, int hSteps = 8, int vSteps = 8)
	{
		vertices.clear();
		indices.clear();
		std::vector<glm::vec3> vectors;

		srand(seed);
		Cylinder(ERRT_RADIUS, hSteps, vSteps, glm::vec3(0.0f), glm::vec3(0.0f, 5.0f, 0.0f));
		vectors.emplace_back(0.0f, 0.0f, 0.0f);
		vectors.emplace_back(0.0f, 5.0f, 0.0f);

		for (int counter = 0; counter < maxCount; counter++)
		{
			int randomIndex = rand() % vectors.size();
			if (randomIndex % 2 == 1) randomIndex--;
			float randomFloat = rand() / (float)RAND_MAX;

			bool xAvailable = glm::abs(glm::dot(glm::normalize(vectors[randomIndex] - vectors[randomIndex + 1]), glm::vec3(1.0f, 0.0f, 0.0f))) < 0.5f;
			bool yAvailable = glm::abs(glm::dot(glm::normalize(vectors[randomIndex] - vectors[randomIndex + 1]), glm::vec3(0.0f, 1.0f, 0.0f))) < 0.5f;
			bool zAvailable = glm::abs(glm::dot(glm::normalize(vectors[randomIndex] - vectors[randomIndex + 1]), glm::vec3(0.0f, 0.0f, -1.0f))) < 0.5f;

			/*
			std::cout << "x: " << xAvailable << '\n';
			std::cout << "y: " << yAvailable << '\n';
			std::cout << "z: " << zAvailable << '\n';
			std::cout << "-----------------------\n";
			*/

			glm::vec3 newPosA = vectors[randomIndex] * randomFloat + vectors[randomIndex + 1] * (1.0f - randomFloat);
			glm::vec3 newPosB;
			int random = rand() % 4;
			if (!yAvailable)
			{
				switch (random)
				{
				case 0:
					newPosB = newPosA + glm::vec3(1.0f, 0.0f, 0.0f);
					break;
				case 1:
					newPosB = newPosA - glm::vec3(1.0f, 0.0f, 0.0f);
					break;
				case 2:
					newPosB = newPosA + glm::vec3(0.0f, 0.0f, -1.0f);
					break;
				case 3:
					newPosB = newPosA - glm::vec3(0.0f, 0.0f, -1.0f);
					break;
				}
			}
			else if (!xAvailable)
			{
				switch (random)
				{
				case 0:
					newPosB = newPosA + glm::vec3(0.0f, 1.0f, 0.0f);
					break;
				case 1:
					newPosB = newPosA - glm::vec3(0.0f, 1.0f, 0.0f);
					break;
				case 2:
					newPosB = newPosA + glm::vec3(0.0f, 0.0f, -1.0f);
					break;
				case 3:
					newPosB = newPosA - glm::vec3(0.0f, 0.0f, -1.0f);
					break;
				}
			}
			else if (!zAvailable)
			{
				switch (random)
				{
				case 0:
					newPosB = newPosA + glm::vec3(0.0f, 1.0f, 0.0f);
					break;
				case 1:
					newPosB = newPosA - glm::vec3(0.0f, 1.0f, 0.0f);
					break;
				case 2:
					newPosB = newPosA + glm::vec3(1.0f, 0.0f, 0.0f);
					break;
				case 3:
					newPosB = newPosA - glm::vec3(1.0f, 0.0f, 0.0f);
					break;
				}
			}

			Cylinder(ERRT_RADIUS, hSteps, vSteps, newPosA, newPosB);
			vectors.push_back(newPosA);
			vectors.push_back(newPosB);
		}

		mesh.vertexBufferLayout = sf::BufferLayout({sf::BufferComponent::VertexPosition});
		mesh.vertexBuffer = malloc(vertices.size() * sizeof(glm::vec3));
		memcpy(mesh.vertexBuffer, vertices.data(), vertices.size() * sizeof(glm::vec3));
		mesh.vertexCount = vertices.size();
		mesh.indexBuffer = new uint32_t[indices.size() + 1];
		memcpy(mesh.indexBuffer, indices.data(), indices.size() * sizeof(uint32_t));
		mesh.indexCount = indices.size();
		mesh.pieces = mesh.indexBuffer + indices.size();
		mesh.pieces[0] = 0;
		mesh.pieceCount = 1;
	}
}