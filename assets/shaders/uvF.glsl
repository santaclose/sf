
in vec2 fTexCoords;
out vec4 outColor;

void main()
{
	outColor = vec4(fTexCoords.x, fTexCoords.y, 0.0, 1.0);
}