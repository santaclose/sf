#version 430 core

in vec2 screenPos;
out vec4 color;

float random(vec2 st) {
	return fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123);
}

void main()
{
	color = vec4(vec3(random(screenPos)), 1.0);
}