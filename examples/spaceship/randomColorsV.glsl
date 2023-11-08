out vec4 fColor;

float random(vec2 st) {
	return fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123);
}

void main()
{
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(VA_POSITION, 1.0);
	fColor = vec4(random(vec2(VA_POSITION.x, VA_POSITION.y)), random(vec2(VA_POSITION.y, VA_POSITION.x)), random(vec2(VA_POSITION.x * 2.0, VA_POSITION.y)), 1.0);
}