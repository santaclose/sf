layout(location = 4) in vec2 fTexCoords;

void main()
{
	OUT_COLOR = vec4(fTexCoords.x, fTexCoords.y, 0.0, 1.0);
}