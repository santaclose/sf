
out vec2 fTexCoords;

void main()
{
	fTexCoords = VA_TEX_COORDS;
	gl_Position = SCREEN_SPACE_MATRIX * vec4(VA_POSITION.xy, 0.0, 1.0);
}