/*#include <vector>
#include "Vertex.h"
#include <iostream>
*/
namespace ModelPrimitives
{
	inline void Plane(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, float size)
	{
		vertexVector.reserve(4);
		indexVector.reserve(6);

		Vertex ok(glm::vec3(-size * 0.5, 0.0, size * 0.5));
		vertexVector.push_back(ok);
		ok.position.x += size;
		vertexVector.push_back(ok);
		ok.position.z -= size;
		vertexVector.push_back(ok);
		ok.position.x -= size;
		vertexVector.push_back(ok);

		indexVector.push_back(vertexVector.size() - 4);//4-0
		indexVector.push_back(vertexVector.size() - 3);//4-1
		indexVector.push_back(vertexVector.size() - 2);//4-2
		indexVector.push_back(vertexVector.size() - 2);//4-2
		indexVector.push_back(vertexVector.size() - 1);//4-3
		indexVector.push_back(vertexVector.size() - 4);//4-0
	}

	inline void Cube(std::vector<Vertex>& vertexVector, std::vector<unsigned int>& indexVector, float size)
	{
		//std::cout << "cube size " << size << std::endl;
		vertexVector.reserve(8);
		indexVector.reserve(36);

		Vertex ok(glm::vec3(-size * 0.5, -size * 0.5, size * 0.5));
		vertexVector.push_back(ok);
		ok.position.x += size;
		vertexVector.push_back(ok);
		ok.position.y += size;
		vertexVector.push_back(ok);
		ok.position.x -= size;
		vertexVector.push_back(ok);
		ok.position.z -= size;
		ok.position.y -= size;
		vertexVector.push_back(ok);
		ok.position.x += size;
		vertexVector.push_back(ok);
		ok.position.y += size;
		vertexVector.push_back(ok);
		ok.position.x -= size;
		vertexVector.push_back(ok);

		indexVector.push_back(vertexVector.size() - 8);//8-0
		indexVector.push_back(vertexVector.size() - 7);//8-1
		indexVector.push_back(vertexVector.size() - 6);//8-2
		indexVector.push_back(vertexVector.size() - 6);//8-2
		indexVector.push_back(vertexVector.size() - 5);//8-3
		indexVector.push_back(vertexVector.size() - 8);//8-0

		indexVector.push_back(vertexVector.size() - 7);//8-1
		indexVector.push_back(vertexVector.size() - 3);//8-5
		indexVector.push_back(vertexVector.size() - 2);//8-6
		indexVector.push_back(vertexVector.size() - 2);//8-6
		indexVector.push_back(vertexVector.size() - 6);//8-2
		indexVector.push_back(vertexVector.size() - 7);//8-1

		indexVector.push_back(vertexVector.size() - 4);//8-4
		indexVector.push_back(vertexVector.size() - 1);//8-7
		indexVector.push_back(vertexVector.size() - 2);//8-6
		indexVector.push_back(vertexVector.size() - 2);//8-6
		indexVector.push_back(vertexVector.size() - 3);//8-5
		indexVector.push_back(vertexVector.size() - 4);//8-4

		indexVector.push_back(vertexVector.size() - 8);//8-0
		indexVector.push_back(vertexVector.size() - 5);//8-3
		indexVector.push_back(vertexVector.size() - 1);//8-7
		indexVector.push_back(vertexVector.size() - 1);//8-7
		indexVector.push_back(vertexVector.size() - 4);//8-4
		indexVector.push_back(vertexVector.size() - 8);//8-0

		indexVector.push_back(vertexVector.size() - 5);//8-3
		indexVector.push_back(vertexVector.size() - 6);//8-2
		indexVector.push_back(vertexVector.size() - 2);//8-6
		indexVector.push_back(vertexVector.size() - 2);//8-6
		indexVector.push_back(vertexVector.size() - 1);//8-7
		indexVector.push_back(vertexVector.size() - 5);//8-3

		indexVector.push_back(vertexVector.size() - 3);//8-5
		indexVector.push_back(vertexVector.size() - 4);//8-4
		indexVector.push_back(vertexVector.size() - 8);//8-0
		indexVector.push_back(vertexVector.size() - 8);//8-0
		indexVector.push_back(vertexVector.size() - 7);//8-1
		indexVector.push_back(vertexVector.size() - 3);//8-5
	}
}