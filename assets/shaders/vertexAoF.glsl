
out vec4 outColor;

in float fVertexAo;

void main()
{
	outColor = vec4(vec3(pow(fVertexAo, 1.0 / 2.2)), 1.0);
}