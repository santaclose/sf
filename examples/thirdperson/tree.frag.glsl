layout(location = 4) in vec2 fTexCoords;
layout(location = 5) in float fVertexAo;

uniform sampler2D texSampler;

void main()
{
	vec4 color = texture(texSampler, fTexCoords);
	// if (color.a < 0.5)
		// discard;
	// OUT_COLOR = vec4(fVertexAo * color.rgb, 1.0);

	// float wantedAlpha = (color.a - 0.5) / max(fwidth(color.a), 0.0001) + 0.5;
	// float wantedAlpha = color.a < 0.5 ? 0.0 : 1.0;

float width = max(fwidth(color.a), 0.0001);
float wantedAlpha = clamp(
    (color.a - 0.5) / width + 0.5,
    0.0,
    1.0
);
	
	OUT_COLOR = vec4(fVertexAo * color.rgb, wantedAlpha);

	// OUT_COLOR = vec4(fTexCoords, 0.0, 1.0);
	// OUT_COLOR = vec4(vec3(pow(fVertexAo, 1.0 / 2.2)), 1.0);
}