
out mat3 fTBN;
out vec3 fWorldPos;
out vec2 fTexCoords;
out float fVertexAo;

uniform bool animate = false;

void main()
{
	fTexCoords = VA_TEX_COORDS;
	fVertexAo = VA_AMBIENT_OCCLUSION;

	mat4 skinMat = animate ?
		VA_BONE_WEIGHTS.x * SKINNING_MATRICES[int(VA_BONE_INDICES.x)] +
		VA_BONE_WEIGHTS.y * SKINNING_MATRICES[int(VA_BONE_INDICES.y)] +
		VA_BONE_WEIGHTS.z * SKINNING_MATRICES[int(VA_BONE_INDICES.z)] +
		VA_BONE_WEIGHTS.w * SKINNING_MATRICES[int(VA_BONE_INDICES.w)] : mat4(1.0);

	vec3 T = normalize(vec3(OBJECT_MATRIX * skinMat * vec4(VA_TANGENT, 0.0)));
	vec3 B = normalize(vec3(OBJECT_MATRIX * skinMat * vec4(VA_BITANGENT, 0.0)));
	vec3 N = normalize(vec3(OBJECT_MATRIX * skinMat * vec4(VA_NORMAL, 0.0)));
	fTBN = mat3(T, B, N);

	// standard normal passing
	//normal = (OBJECT_MATRIX * vec4(VA_NORMAL, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(OBJECT_MATRIX))* VA_NORMAL).xyz;

	fWorldPos = (OBJECT_MATRIX * skinMat * vec4(VA_POSITION, 1.0)).rgb;
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * skinMat * vec4(VA_POSITION, 1.0);
}