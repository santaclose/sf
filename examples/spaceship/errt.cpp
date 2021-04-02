#include <ml.h>
#include <iostream>
#include <math.h>
#include <vector>


#define RADIUS 0.05
namespace sf::Models
{
	int seed = 0;
	int cylinderRes = 8;
	int maxCount = 40;

	void Cylinder(float radius, int steps, const sfmg::vl::vec& posA, const sfmg::vl::vec& posB, bool cap = true)
	{
		using namespace sfmg::ml;
		using namespace sfmg::vl;
		vec dir = (posB - posA);
		dir.Normalize();
		float test = dir.Dot(vec::up);
		vec localRight = abs(test) == 1.0 ? vec::right : dir * vec::up;
		vec localForward = (dir * localRight).Normalized();

		const float PI = 3.14159265358;

		float angleStep = 2.0 * PI / (double)steps;
		unsigned int quad[4];
		unsigned int* vertices = (unsigned int*)alloca(sizeof(unsigned int) * steps * 2);

		unsigned int* capA = (unsigned int*)alloca(sizeof(unsigned int) * steps);
		unsigned int* capB = (unsigned int*)alloca(sizeof(unsigned int) * steps);

		for (int i = 0; i < steps; i++)
		{
			vertices[i * 2] = capA[i] = vertex(posA + localRight * cos(angleStep * i) * radius + localForward * sin(angleStep * i) * radius);
			vertices[i * 2 + 1] = capB[i] = vertex(posB + localRight * cos(angleStep * i) * radius + localForward * sin(angleStep * i) * radius);

			if (i > 0)
			{
				quad[0] = vertices[i * 2 - 2];
				quad[1] = vertices[i * 2];
				quad[2] = vertices[i * 2 + 1];
				quad[3] = vertices[i * 2 - 1];
				face(quad, 4);
			}
		}
		quad[0] = quad[1];
		quad[3] = quad[2];
		quad[1] = vertices[0];
		quad[2] = vertices[1];
		face(quad, 4);

		if (!cap)
			return;

		face(capA, steps, true);
		face(capB, steps, false);
	}
	void Cylinder(float radius, int steps, const sfmg::vl::vec&& posA, const sfmg::vl::vec&& posB, bool cap = true)
	{
		using namespace sfmg::vl;
		vec a = posA;
		vec b = posB;
		Cylinder(radius, steps, a, b, cap);
	}

	void GenerateModel()
	{
		using namespace sfmg::vl;
		std::vector<vec> vectors;

		srand(seed);
		Cylinder(RADIUS, cylinderRes, vec::zero, vec(0, 5, 0));
		vectors.push_back(vec::zero);
		vectors.emplace_back(0, 5, 0);

		for (int counter = 0; counter < maxCount; counter++)
		{
			int randomIndex = rand() % vectors.size();
			if (randomIndex % 2 == 1) randomIndex--;
			float randomFloat = rand() / (float)RAND_MAX;

			bool xAvailable = abs((vectors[randomIndex] - vectors[randomIndex + 1]).Normalized().Dot(vec::right)) < 0.5;
			bool yAvailable = abs((vectors[randomIndex] - vectors[randomIndex + 1]).Normalized().Dot(vec::up)) < 0.5;
			bool zAvailable = abs((vectors[randomIndex] - vectors[randomIndex + 1]).Normalized().Dot(vec::forward)) < 0.5;

			/*
			std::cout << "x: " << xAvailable << '\n';
			std::cout << "y: " << yAvailable << '\n';
			std::cout << "z: " << zAvailable << '\n';
			std::cout << "-----------------------\n";
			*/

			vec newPosA = vectors[randomIndex] * randomFloat + vectors[randomIndex + 1] * (1.0 - randomFloat);
			vec newPosB;
			int random = rand() % 4;
			if (!yAvailable)
			{
				switch (random)
				{
				case 0:
					newPosB = newPosA + vec::right;
					break;
				case 1:
					newPosB = newPosA - vec::right;
					break;
				case 2:
					newPosB = newPosA + vec::forward;
					break;
				case 3:
					newPosB = newPosA - vec::forward;
					break;
				}
			}
			else if (!xAvailable)
			{
				switch (random)
				{
				case 0:
					newPosB = newPosA + vec::up;
					break;
				case 1:
					newPosB = newPosA - vec::up;
					break;
				case 2:
					newPosB = newPosA + vec::forward;
					break;
				case 3:
					newPosB = newPosA - vec::forward;
					break;
				}
			}
			else if (!zAvailable)
			{
				switch (random)
				{
				case 0:
					newPosB = newPosA + vec::up;
					break;
				case 1:
					newPosB = newPosA - vec::up;
					break;
				case 2:
					newPosB = newPosA + vec::right;
					break;
				case 3:
					newPosB = newPosA - vec::right;
					break;
				}
			}

			Cylinder(RADIUS, cylinderRes, newPosA, newPosB);
			vectors.push_back(newPosA);
			vectors.push_back(newPosB);
		}
	}
}