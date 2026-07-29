#pragma once

#include <Components/Transform.h>

namespace sf
{
	struct TerrainCollider
	{
		glm::vec3 aabbMin;
		glm::vec3 aabbMax;
		const Bitmap* bitmap = nullptr;
		inline TerrainCollider ApplyTransform(const Transform& transform)
		{
			assert(transform.rotation == glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));
			TerrainCollider out;
			out.aabbMin = transform.ApplyToPoint(aabbMin);
			out.aabbMax = transform.ApplyToPoint(aabbMax);
			out.bitmap = bitmap;
			return out;
		}

		inline bool Sample(const glm::vec3& point, float& outHeight) const
		{
			float maxHeight = aabbMax.y - aabbMin.y;
			glm::vec2 size = glm::vec2(aabbMax.x - aabbMin.x, aabbMax.z - aabbMin.z);
			glm::vec2 heightmapUV;
			heightmapUV.x = (point.x - aabbMin.x) / size.x;
			heightmapUV.y = (point.z - aabbMin.z) / size.y;
			heightmapUV.x = heightmapUV.x * ((float)(bitmap->width - 1) / (float)bitmap->width) + 0.5f / (float)bitmap->width;
			heightmapUV.y = heightmapUV.y * ((float)(bitmap->height - 1) / (float)bitmap->height) + 0.5f / (float)bitmap->height;
			if (heightmapUV.x > 0.0f && heightmapUV.x < 1.0f && heightmapUV.y > 0.0f && heightmapUV.y < 1.0f)
			{
				float heightSample = bitmap->Sample<uint16_t>(heightmapUV);
				outHeight = heightSample * maxHeight;
				return true;
			}
			return false;
		}
	};
}