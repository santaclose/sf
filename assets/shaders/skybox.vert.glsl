layout(location = 0) out vec3 fTexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	fTexCoords = VA_Position;
	gl_Position = projection * view * vec4(VA_Position, 1.0);
}