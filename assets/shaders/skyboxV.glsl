
out vec3 fTexCoords;

void main()
{
	fTexCoords = VA_POSITION;
	gl_Position = SKYBOX_MATRIX * vec4(VA_POSITION, 1.0);
}