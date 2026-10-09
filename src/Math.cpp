#include "Math.hpp"

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

void sf::Math::PointWeighting2D(const glm::vec2& sample, const glm::vec2* points, float* outWeights, int count, int pointStride)
{
	assert(count > 0);
#define POINT(i) ((const glm::vec2*)(((const uint8_t*)points) + (pointStride * (i))))[0]
	static std::unordered_map<const glm::vec2*, float*> blendMatrices;
	if (blendMatrices.find(points) == blendMatrices.end())
	{
		blendMatrices[points] = new float[count * count];
		BlendMatrixInterpolation::slice2d<float> blendMatrixSlice2d(count, count, blendMatrices[points]);
		glm::vec2* tempPointArray = (glm::vec2*)alloca(sizeof(glm::vec2) * count);
		for (int i = 0; i < count; i++)
			tempPointArray[i] = POINT(i);
		BlendMatrixInterpolation::fit_blend_matrix<glm::vec2>(blendMatrixSlice2d, tempPointArray, count);
	}

	BlendMatrixInterpolation::slice2d<float> blendMatrixSlice2d(count, count, blendMatrices[points]);
	float* distances = (float*)alloca(sizeof(float) * count);
	for (int i = 0; i < count; i++)
		distances[i] = glm::length(POINT(i) - sample);
	BlendMatrixInterpolation::compute_blend_weights(BlendMatrixInterpolation::slice1d<float>(count, outWeights), BlendMatrixInterpolation::slice1d<float>(count, distances), blendMatrixSlice2d);
#undef POINT
}