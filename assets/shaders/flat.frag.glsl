layout(location = 4) in vec2 fTexCoords;

uniform bool useTexture = false;
uniform sampler2D texture;
uniform vec3 color;

void main()
{
	vec3 texColor =
		texture2D(texture, fTexCoords).rgb * float(useTexture) + 
		color * float(!useTexture);
	OUT_COLOR = vec4(texColor, 1.0);
}