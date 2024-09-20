#version 450

layout(location = 0) in vec2 fScreenPos;
layout(location = 0) out vec4 outColor;

float random(vec2 st) {
	return fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123);
}

void main()
{
	outColor = vec4(vec3(random(fScreenPos)), 1.0);
}