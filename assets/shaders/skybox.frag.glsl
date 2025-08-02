layout(location = 0) in vec3 fTexCoords;

uniform samplerCube skybox;

void main()
{
	OUT_COLOR = texture(skybox, fTexCoords);
}