
uniform sampler2D atlas;

in vec2 fTexCoords;

out vec4 outColor;

float median(float a, float b, float c)
{
	return max(min(a, b), min(max(a, b), c));
}

void main()
{
	vec4 insideColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 outsideColor = vec4(1.0, 1.0, 1.0, 1.0);

	// Bilinear sampling of the distance field
	vec3 s = texture2D(atlas, fTexCoords).rgb;
	// Acquiring the signed distance
	float d = median(s.r, s.g, s.b) - 0.5;
	// The anti-aliased measure of how "inside" the fragment lies
	float w = clamp(d / fwidth(d) + 0.5, 0.0, 1.0);
	// Combining the two colors
	outColor = mix(outsideColor, insideColor, w);
}