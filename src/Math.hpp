#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace sf {

	inline glm::vec3 operator/(const glm::vec3& v, const float& f)
	{
		return glm::vec3(v.x / f, v.y / f, v.z / f);
	}
	inline glm::vec3 operator*(const glm::vec3& v, const float& f)
	{
		return glm::vec3(v.x * f, v.y * f, v.z * f);
	}

	namespace Math {

		constexpr static float Pi = 3.14159265358979323846264;
		constexpr static float DTOR = 3.14159265358979323846264 / 180.0;
		constexpr static float RTOD = 180.0 / 3.14159265358979323846264;

		inline int Mod(int a, int b)
		{
			int ret = a % b;
			if (ret < 0)
				ret += b;
			return ret;
		}

		inline void WeightedBlend(const float* input, const float* weights, int count, float& output)
		{
			assert(count > 1);

			output = 0.0f;
			for (int i = 0; i < count; i++)
				output += input[i] * weights[i];
		}

		inline void WeightedBlend(const glm::vec2* input, const float* weights, int count, glm::vec2& output)
		{
			assert(count > 1);

			output = { 0.0f, 0.0f };
			for (int i = 0; i < count; i++)
				output += input[i] * weights[i];
		}

		inline void WeightedBlend(const glm::vec3* input, const float* weights, int count, glm::vec3& output)
		{
			assert(count > 1);

			output = { 0.0f, 0.0f, 0.0f };
			for (int i = 0; i < count; i++)
				output += input[i] * weights[i];
		}

		inline void WeightedBlend(const glm::quat* input, const float* weights, int count, glm::quat& output)
		{
			assert(count > 1);

			float total = weights[0];
			output = input[0];
			for (int i = 1; i < count; i++)
			{
				total += weights[i];
				bool totalIsZero = glm::abs(total) < 0.0000001f;
				output = glm::slerp(output, input[i], totalIsZero ? weights[i] : weights[i] / total);
			}
		}
	}
}