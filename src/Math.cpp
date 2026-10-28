#include "Math.hpp"

#include <queue>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <BlendMatrixInterpolation.h>

void sf::Math::PointWeighting1D(float sample, const float* points, float* outWeights, int count, int pointStride)
{
	assert(count > 0);
#define POINT(i) ((const float*)(((const uint8_t*)points) + (pointStride * (i))))[0]
	sample = glm::clamp(sample, POINT(0), POINT(count - 1));
	for (int i = 0; i < count; i++)
	{
		assert(i == 0 || POINT(i) > POINT(i - 1));
		if (i < count - 1 && POINT(i) < sample && POINT(i + 1) > sample)
		{
			float distance = POINT(i + 1) - POINT(i);
			outWeights[i + 1] = (sample - POINT(i)) / distance;
			outWeights[i] = 1.0f - outWeights[i + 1];
			i++;
		}
		else if (POINT(i) == sample)
			outWeights[i] = 1.0f;
		else
			outWeights[i] = 0.0f;
	}
#undef POINT
}

void sf::Math::PointWeightingBlendMatrix2D(const glm::vec2& sample, const glm::vec2* points, const float* blendMatrix, float* outWeights, int count, int pointStride)
{
	assert(count > 0);
	static std::vector<float> distances;
	distances.resize(count);
#define POINT(i) ((const glm::vec2*)(((const uint8_t*)points) + (pointStride * (i))))[0]
	for (int i = 0; i < count; i++)
		distances[i] = glm::length(POINT(i) - sample);
#undef POINT
	BlendMatrixInterpolation::slice2d<float> blendMatrixSlice2d(count, count, (float*)blendMatrix);
	BlendMatrixInterpolation::compute_blend_weights(BlendMatrixInterpolation::slice1d<float>(count, outWeights), BlendMatrixInterpolation::slice1d<float>(count, distances.data()), blendMatrixSlice2d);
}

void sf::Math::PointWeightingBlendMatrix3D(const glm::vec3& sample, const glm::vec3* points, const float* blendMatrix, float* outWeights, int count, int pointStride)
{
	assert(count > 0);
	static std::vector<float> distances;
	distances.resize(count);
#define POINT(i) ((const glm::vec3*)(((const uint8_t*)points) + (pointStride * (i))))[0]
	for (int i = 0; i < count; i++)
		distances[i] = glm::length(POINT(i) - sample);
#undef POINT
	BlendMatrixInterpolation::slice2d<float> blendMatrixSlice2d(count, count, (float*)blendMatrix);
	BlendMatrixInterpolation::compute_blend_weights(BlendMatrixInterpolation::slice1d<float>(count, outWeights), BlendMatrixInterpolation::slice1d<float>(count, distances.data()), blendMatrixSlice2d);
}

void sf::Math::ComputePointWeightBlendMatrix2D(const glm::vec2* points, int count, float* outBlendMatrix, int pointStride)
{
	BlendMatrixInterpolation::slice2d<float> blendMatrixSlice2d(count, count, outBlendMatrix);
	BlendMatrixInterpolation::fit_blend_matrix<glm::vec2>(blendMatrixSlice2d, (glm::vec2*) points, count, pointStride);
}

void sf::Math::ComputePointWeightBlendMatrix3D(const glm::vec3* points, int count, float* outBlendMatrix, int pointStride)
{
	BlendMatrixInterpolation::slice2d<float> blendMatrixSlice2d(count, count, outBlendMatrix);
	BlendMatrixInterpolation::fit_blend_matrix<glm::vec3>(blendMatrixSlice2d, (glm::vec3*) points, count, pointStride);
}

void sf::Math::ClosestPoints(const glm::vec3& target, const glm::vec3* points, int count, int wantedCount, std::vector<glm::vec3>& outPoints, std::vector<uint32_t>& outIndices, int pointStride)
{
#define POINT(i) ((const glm::vec3*)(((const uint8_t*)points) + (pointStride * (i))))[0]

	if (wantedCount <= 0) return;
	if (wantedCount > count) wantedCount = count;

	// Max-Heap tracking: <squared_distance, point_index>
	std::priority_queue<std::pair<float, size_t>> maxHeap;

	for (size_t i = 0; i < count; ++i)
	{
		float distSq = glm::distance2(POINT(i), target);

		if (maxHeap.size() < wantedCount)
			maxHeap.push({distSq, i});
		else if (distSq < maxHeap.top().first)
		{
			maxHeap.pop();
			maxHeap.push({distSq, i});
		}
	}

	// Unpack heap into a vector (returns closest to farthest order via reversing)
	outPoints.resize(maxHeap.size());
	outIndices.resize(maxHeap.size());
	for (int i = static_cast<int>(maxHeap.size()) - 1; i > -1; i--)
	{
		outIndices[i] = maxHeap.top().second;
		outPoints[i] = POINT(maxHeap.top().second);
		maxHeap.pop();
	}

#undef POINT
}