layout(location = 5) in float fVertexAo;

void main()
{
	OUT_COLOR = vec4(vec3(pow(fVertexAo, 1.0 / 2.2)), 1.0);
}