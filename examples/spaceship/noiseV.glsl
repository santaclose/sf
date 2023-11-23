
out vec2 fScreenPos;

float random(vec2 st) {
	return fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123);
}

void main()
{
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(VA_POSITION + normalize(vec3(
		random(CAMERA_MATRIX[0].xy + vec2(float(gl_VertexID))) - 0.5,
		random(CAMERA_MATRIX[1].xy + vec2(float(gl_VertexID))) - 0.5,
		random(CAMERA_MATRIX[2].xy + vec2(float(gl_VertexID))) - 0.5
		))*.02, 1.0);
	fScreenPos = gl_Position.xy;
}