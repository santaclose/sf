#version 330 core

uniform sampler2D atlas;
in vec2 texCoord;
in mat3 TBN;
out vec4 color;

float median(float a, float b, float c)
{
	return max(min(a, b), min(max(a, b), c));
}

void main()
{
	vec4 insideColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 outsideColor = vec4(1.0, 1.0, 1.0, 1.0);

	// Bilinear sampling of the distance field
	vec3 s = texture2D(atlas, texCoord).rgb;
	// Acquiring the signed distance
	float d = median(s.r, s.g, s.b) - 0.5;
	// The anti-aliased measure of how "inside" the fragment lies
	float w = clamp(d / fwidth(d) + 0.5, 0.0, 1.0);
	// Combining the two colors
	color = mix(outsideColor, insideColor, w);

}