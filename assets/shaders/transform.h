#include <assets/shaders/quaternion.h>

#define TRANSFORM_IDENTITY Transform(vec3(0.0, 0.0, 0.0), QUATERNION_IDENTITY, 1.0)
#define TRANSFORM_LOAD_FROM_ARRAY(outTransform, index, arr) \
{ \
	int base = (index) * 8; \
	(outTransform).position = vec3( \
		(arr)[base + 0], \
		(arr)[base + 1], \
		(arr)[base + 2] \
	); \
	(outTransform).rotation = vec4( \
		(arr)[base + 3], \
		(arr)[base + 4], \
		(arr)[base + 5], \
		(arr)[base + 6] \
	); \
	(outTransform).scale = (arr)[base + 7]; \
}

struct Transform
{
	vec3 position;
	vec4 rotation;
	float scale;
};

mat4 TransformToMatrix(Transform t)
{
	float x = t.rotation.x, y = t.rotation.y, z = t.rotation.z, w = t.rotation.w;

	// Precompute products
	float x2 = x + x,  y2 = y + y,  z2 = z + z;
	float xx = x * x2, xy = x * y2, xz = x * z2;
	float yy = y * y2, yz = y * z2, zz = z * z2;
	float wx = w * x2, wy = w * y2, wz = w * z2;

	return mat4(
		vec4(t.scale * (1.0 - (yy + zz)), t.scale * (xy + wz),         t.scale * (xz - wy),         0.0),
		vec4(t.scale * (xy - wz),         t.scale * (1.0 - (xx + zz)), t.scale * (yz + wx),         0.0),
		vec4(t.scale * (xz + wy),         t.scale * (yz - wx),         t.scale * (1.0 - (xx + yy)), 0.0),
		vec4(t.position, 1.0)
	);
}

Transform TransformApply(Transform a, Transform b)
{
	Transform newTransform;
	newTransform.position = a.position + QuatRotateVector(a.rotation, b.position * a.scale);
	newTransform.rotation = QuatMultiply(a.rotation, b.rotation);
	newTransform.scale = a.scale * b.scale;
	return newTransform;
}

Transform TransformWeightedBlend(Transform inputTransforms[4], vec4 weights)
{
	Transform outTransform = Transform(vec3(0.0, 0.0, 0.0), QUATERNION_IDENTITY, 0.0);
	outTransform.position += inputTransforms[0].position * weights.x;
	outTransform.position += inputTransforms[1].position * weights.y;
	outTransform.position += inputTransforms[2].position * weights.z;
	outTransform.position += inputTransforms[3].position * weights.w;

	vec4 rotations[4];
	rotations[0] = inputTransforms[0].rotation;
	rotations[1] = inputTransforms[1].rotation;
	rotations[2] = inputTransforms[2].rotation;
	rotations[3] = inputTransforms[3].rotation;
	// outTransform.rotation = QuatLookRotation(vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 0.0));
	outTransform.rotation = QuatBlendAccurate(rotations, weights);
	// float total = weights.x;
	// outTransform.rotation = inputTransforms[0].rotation;
	// for (int i = 1; i < 4; i++)
	// {
	// 	total += weights[i];
	// 	bool totalIsZero = abs(total) < 0.00001;
	// 	outTransform.rotation = QuatSlerp(outTransform.rotation, inputTransforms[i].rotation, totalIsZero ? weights[i] : weights[i] / total);
	// }
	outTransform.scale += inputTransforms[0].scale * weights.x;
	outTransform.scale += inputTransforms[1].scale * weights.y;
	outTransform.scale += inputTransforms[2].scale * weights.z;
	outTransform.scale += inputTransforms[3].scale * weights.w;
	return outTransform;
}