layout(location = 0) in vec3 fColor;

void main()
{
	OUT_COLOR = vec4(fColor, 1.0);
}