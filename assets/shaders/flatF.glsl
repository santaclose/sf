#version 430 core

in vec2 fTexCoords;

uniform bool useTexture = false;
uniform sampler2D texture;
uniform vec3 color;

out vec4 outColor;

void main()
{
	vec3 texColor =
		texture2D(texture, fTexCoords).rgb * float(useTexture) + 
		color * float(!useTexture);
	outColor = vec4(texColor, 1.0);
}