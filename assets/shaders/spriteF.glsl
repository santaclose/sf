
in vec2 fTexCoords;
out vec4 outColor;

uniform sampler2D bitmap;

void main()
{
	outColor = texture(bitmap, fTexCoords);
}