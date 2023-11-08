
out mat3 fTBN;
out vec3 fWorldPos;
out vec2 fTexCoords;
out float fVertexAo;

void main()
{
	fVertexAo = VA_AMBIENT_OCCLUSION;
	fTexCoords = VA_TEX_COORDS;
	fWorldPos = (OBJECT_MATRIX * vec4(VA_POSITION, 1.0)).rgb;

	vec3 T = normalize(vec3(OBJECT_MATRIX * vec4(VA_TANGENT, 0.0)));
	vec3 B = normalize(vec3(OBJECT_MATRIX * vec4(VA_BITANGENT, 0.0)));
	vec3 N = normalize(vec3(OBJECT_MATRIX * vec4(VA_NORMAL, 0.0)));
	fTBN = mat3(T, B, N);

	// standard normal passing
	//normal = (OBJECT_MATRIX * vec4(VA_NORMAL, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(OBJECT_MATRIX))* VA_NORMAL).xyz;

	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(VA_POSITION, 1.0);
}