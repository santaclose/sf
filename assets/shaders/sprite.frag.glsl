layout(location = 4) in vec2 fTexCoords;

uniform sampler2D bitmap;

void main()
{
	OUT_COLOR = texture(bitmap, fTexCoords);
}