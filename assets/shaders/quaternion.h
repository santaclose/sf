#define QUATERNION_IDENTITY vec4(0, 0, 0, 1)
#ifndef PI
#define PI 3.14159265358979323846264
#endif
#ifndef DTOR
#define DTOR (PI/180.0)
#endif

vec3 QuatRotateVector(vec3 v, vec4 q)
{
	vec3 u = vec3(q.r, q.g, q.b);
	float s = q.a;
	return
		2.0f * dot(u, v) * u +
		(s*s - dot(u, u)) * v +
		2.0f * s * cross(u, v);
}

vec4 QuatMultiply(vec4 q1, vec4 q2)
{
	return vec4(
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
		q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z);
}

vec4 QuatFromEuler(vec3 angles)
{
	float cr = cos(angles.x * 0.5);
	float sr = sin(angles.x * 0.5);
	float cp = cos(angles.y * 0.5);
	float sp = sin(angles.y * 0.5);
	float cy = cos(angles.z * 0.5);
	float sy = sin(angles.z * 0.5);
	vec4 q;
	q.w = cr * cp * cy + sr * sp * sy;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;
	return q;
}

vec4 QuatConjugate(vec4 q)
{
	return vec4(-q.x, -q.y, -q.z, q.w);
}

vec4 QuatInverse(vec4 q)
{
	return QuatConjugate(q) / dot(q, q);
}

mat3 QuatToMat3(vec4 q)
{
	mat3 Result = mat3(1);
	float qxx = q.x * q.x;
	float qyy = q.y * q.y;
	float qzz = q.z * q.z;
	float qxz = q.x * q.z;
	float qxy = q.x * q.y;
	float qyz = q.y * q.z;
	float qwx = q.w * q.x;
	float qwy = q.w * q.y;
	float qwz = q.w * q.z;

	Result[0][0] = 1.0f - 2.0f * (qyy + qzz);
	Result[0][1] = 2.0f * (qxy + qwz);
	Result[0][2] = 2.0f * (qxz - qwy);

	Result[1][0] = 2.0f * (qxy - qwz);
	Result[1][1] = 1.0f - 2.0f * (qxx + qzz);
	Result[1][2] = 2.0f * (qyz + qwx);

	Result[2][0] = 2.0f * (qxz + qwy);
	Result[2][1] = 2.0f * (qyz - qwx);
	Result[2][2] = 1.0f - 2.0f * (qxx + qyy);
	return Result;
}

vec4 QuatFromMat3(mat3 m)
{
	float fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
	float fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
	float fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
	float fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

	int biggestIndex = 0;
	float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	float biggestVal = sqrt(fourBiggestSquaredMinus1 + 1.0) * 0.5;
	float mult = 0.25 / biggestVal;

	if (biggestIndex == 0)
		return vec4((m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult, biggestVal);
	if (biggestIndex == 1)
		return vec4(biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] - m[2][1]) * mult);
	if (biggestIndex == 2)
		return vec4((m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult, (m[2][0] - m[0][2]) * mult);
	else
		return vec4((m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal, (m[0][1] - m[1][0]) * mult);
}

vec4 QuatLookRotation(vec3 direction, vec3 up)
{
	mat3 result;
	result[2] = -direction;
	vec3 right =  cross(up, result[2]);
	result[0] = right * inversesqrt(max(0.00001, dot(right, right)));
	result[1] = cross(result[2], result[0]);
	return QuatFromMat3(result);
}

vec4 QuatSlerp(vec4 q1, vec4 q2, float t)
{
	vec4 temp = q2;
	float cosTheta = dot(q1, q2);

	if (cosTheta < 0.0)
	{
		temp = -q2;
		cosTheta = -cosTheta;
	}

	if (cosTheta > 1.0 - 0.00001)
	{
		return vec4(
			mix(q1.x, temp.x, t),
			mix(q1.y, temp.y, t),
			mix(q1.z, temp.z, t),
			mix(q1.w, temp.w, t));
	}
	else
	{
		float angle = acos(cosTheta);
		return (sin((1.0 - t) * angle) * q1 + sin(t * angle) * temp) / sin(angle);
	}
}