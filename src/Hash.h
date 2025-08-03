#pragma once

#include <glm/glm.hpp>
#include <functional>

namespace sf::Hash
{
	unsigned SimpleStringHash(const char* s);

	struct UVec3Hash {
		inline uint64_t operator()(const glm::uvec3& v) const noexcept {
			// Basic hash combiner (from Boost)
			uint64_t h = std::hash<uint32_t>{}(v.x);
			h ^= std::hash<uint32_t>{}(v.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= std::hash<uint32_t>{}(v.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};
}