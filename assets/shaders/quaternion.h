vec3 RotateVector(vec3 v, vec4 q)
{
	// Extract the vector part of the quaternion
	vec3 u = vec3(q.r, q.g, q.b);

	// Extract the scalar part of the quaternion
	float s = q.a;

	// Do the math
	return 2.0f * dot(u, v) * u
		   + (s*s - dot(u, u)) * v
		   + 2.0f * s * cross(u, v);
}
vec4 RotateQuaternion(vec4 q1, vec4 q2) {
	return vec4(q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
				q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
				q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
				q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z);
}
vec4 QuaternionFromEuler(vec3 angles)
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