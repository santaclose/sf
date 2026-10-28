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

		inline void WeightedBlend(const float* input, const float* weights, int count, float& output, uint32_t inputStride = sizeof(float))
		{
			assert(count > 1);
#define INPUT(i) ((const float*)(((const uint8_t*)input) + (inputStride * (i))))[0]

			output = 0.0f;
			for (int i = 0; i < count; i++)
				output += INPUT(i) * weights[i];
#undef INPUT
		}

		template <typename T>
		inline void WeightedBlend(const T* input, const float* weights, int count, T& output, uint32_t inputStride = sizeof(T))
		{
			assert(count > 1);
#define INPUT(i) ((const T*)(((const uint8_t*)input) + (inputStride * (i))))[0]

			output = T(0.0f);
			for (int i = 0; i < count; i++)
				output += INPUT(i) * weights[i];
#undef INPUT
		}

		inline void WeightedBlend(const glm::quat* input, const float* weights, int count, glm::quat& output, uint32_t inputStride = sizeof(glm::quat))
		{
			assert(count > 1);
#define INPUT(i) ((const glm::quat*)(((const uint8_t*)input) + (inputStride * (i))))[0]

			float total = weights[0];
			output = INPUT(0);
			for (int i = 1; i < count; i++)
			{
				total += weights[i];
				bool totalIsZero = glm::abs(total) < 0.0000001f;
				output = glm::slerp(output, INPUT(i), totalIsZero ? weights[i] : weights[i] / total);
			}
#undef INPUT
		}

		void PointWeighting1D(float sample, const float* points, float* outWeights, int count, int pointStride = sizeof(float));
		void PointWeightingBlendMatrix2D(const glm::vec2& sample, const glm::vec2* points, const float* blendMatrix, float* outWeights, int count, int pointStride = sizeof(glm::vec2));
		void PointWeightingBlendMatrix3D(const glm::vec3& sample, const glm::vec3* points, const float* blendMatrix, float* outWeights, int count, int pointStride = sizeof(glm::vec3));

		void ComputePointWeightBlendMatrix2D(const glm::vec2* points, int count, float* outBlendMatrix, int pointStride = sizeof(glm::vec2));
		void ComputePointWeightBlendMatrix3D(const glm::vec3* points, int count, float* outBlendMatrix, int pointStride = sizeof(glm::vec3));

		void ClosestPoints(const glm::vec3& target, const glm::vec3* points, int count, int wantedCount, std::vector<glm::vec3>& outPoints, std::vector<uint32_t>& outIndices, int pointStride = sizeof(glm::vec3));

		inline uint32_t NextPowerOf2(uint32_t v)
		{
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;
			return v;
		}

		inline uint32_t CountTrailingZeroes(uint32_t v)
		{
			uint32_t i = 0;
			for (; i < 32 && !((v >> i) & 1); ++i);
			return i;
		}

		inline float Cross2D(const glm::vec2& a, const glm::vec2& b)
		{
			return a.x * b.y - a.y * b.x;
		}
	}
}