#include "Math.hpp"

#define EPSILON 0.001f

glm::vec3 sf::operator/(const glm::vec3& v, const float& f)
{
	return glm::vec3(v.x / f, v.y / f, v.z / f);
}
glm::vec3 sf::operator*(const glm::vec3& v, const float& f)
{
	return glm::vec3(v.x * f, v.y * f, v.z * f);
}

const float sf::Math::Pi = 3.14159265358979323846264;

int sf::Math::Mod(int a, int b)
{
	int ret = a % b;
	if (ret < 0)
		ret += b;
	return ret;
}

void sf::Math::WeightedBlend(const float* input, const float* weights, int count, float& output)
{
	assert(count > 1);

	output = 0.0f;
	for (int i = 0; i < count; i++)
		output += input[i] * weights[i];
}

void sf::Math::WeightedBlend(const glm::vec2* input, const float* weights, int count, glm::vec2& output)
{
	assert(count > 1);

	output = { 0.0f, 0.0f };
	for (int i = 0; i < count; i++)
		output += input[i] * weights[i];
}

void sf::Math::WeightedBlend(const glm::vec3* input, const float* weights, int count, glm::vec3& output)
{
	assert(count > 1);

	output = { 0.0f, 0.0f, 0.0f };
	for (int i = 0; i < count; i++)
		output += input[i] * weights[i];
}

void sf::Math::WeightedBlend(const glm::quat* input, const float* weights, int count, glm::quat& output)
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

bool sf::Math::RayTriIntersect(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& t0, const glm::vec3& t1, const glm::vec3& t2, float* out_t)
{
	glm::vec3 normal = glm::cross(t1 - t0, t2 - t0);
	float q = glm::dot(normal, dir);
	if (q == 0) return false;

	float d = -glm::dot(normal, t0);
	float t = -(glm::dot(normal, origin) + d) / q;
	if (t < 0) return false;

	glm::vec3 hit_point = origin + dir * t;

	glm::vec3 edge0 = t1 - t0;
	glm::vec3 VP0 = hit_point - t0;
	if (glm::dot(normal, glm::cross(edge0, VP0)) < 0)
		return false;

	glm::vec3 edge1 = t2 - t1;
	glm::vec3 VP1 = hit_point - t1;
	if (glm::dot(normal, glm::cross(edge1, VP1)) < 0)
		return false;

	glm::vec3 edge2 = t0 - t2;
	glm::vec3 VP2 = hit_point - t2;
	if (glm::dot(normal, glm::cross(edge2, VP2)) < 0)
		return false;

	if (out_t) *out_t = t;
	return true;
}

bool sf::Math::RayAABBIntersect(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& min, const glm::vec3& max, glm::vec3* out)
{
	glm::vec3 dirfrac;

	dirfrac.x = 1.0f / (dir.x == 0 ? 0.00000001f : dir.x);
	dirfrac.y = 1.0f / (dir.y == 0 ? 0.00000001f : dir.y);
	dirfrac.z = 1.0f / (dir.z == 0 ? 0.00000001f : dir.z);

	float t1 = (min.x - origin.x) * dirfrac.x;
	float t2 = (max.x - origin.x) * dirfrac.x;
	float t3 = (min.y - origin.y) * dirfrac.y;
	float t4 = (max.y - origin.y) * dirfrac.y;
	float t5 = (min.z - origin.z) * dirfrac.z;
	float t6 = (max.z - origin.z) * dirfrac.z;

	float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
	float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

	if (tmax < 0 || tmin > tmax)
		return false;

	if (out != nullptr)
		*out = tmin < 0 ? origin : origin + dir * tmin;
	return true;
}

float sf::Math::LineSegmentPointDistance2D(const glm::vec2& p, const glm::vec2& l0, const glm::vec2& l1)
{
	float l2 = glm::pow(l0.x - l1.x, 2.0f) + glm::pow(l0.y - l1.y, 2.0f);
	if (l2 == 0.0f)
		return glm::pow(p.x - l0.x, 2.0f) + glm::pow(p.y - l0.y, 2.0f);
	float t = ((p.x - l0.x) * (l1.x - l0.x) + (p.y - l0.y) * (l1.y - l0.y)) / l2;
	t = glm::max(0.0f, glm::min(1.0f, t));
	glm::vec2 o = { l0.x + t * (l1.x - l0.x), l0.y + t * (l1.y - l0.y) };
	return glm::sqrt(glm::pow(p.x - o.x, 2.0f) + glm::pow(p.y - o.y, 2.0f));
}

bool sf::Math::TriPointIntersect2D(const glm::vec2& p, const glm::vec2& t0, const glm::vec2& t1, const glm::vec2& t2)
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = (p.x - t1.x) * (t0.y - t1.y) - (t0.x - t1.x) * (p.y - t1.y);
	d2 = (p.x - t2.x) * (t1.y - t2.y) - (t1.x - t2.x) * (p.y - t2.y);
	d3 = (p.x - t0.x) * (t2.y - t0.y) - (t2.x - t0.x) * (p.y - t0.y);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

float sf::Math::TriPointDistance2D(const glm::vec2& p, const glm::vec2& t0, const glm::vec2& t1, const glm::vec2& t2)
{
	if (TriPointIntersect2D(p, t0, t1, t2))
		return 0.0f;

	float disToSideA = LineSegmentPointDistance2D(p, t0, t1);
	float disToSideB = LineSegmentPointDistance2D(p, t1, t2);
	float disToSideC = LineSegmentPointDistance2D(p, t2, t0);

	return glm::min(glm::min(disToSideA, disToSideB), disToSideC);
}
