layout(location = 0) in vec2 particleUV;
layout(location = 1) in float particleOpacity;

#define LINE_WIDTH 0.05
void main()
{
	float finalOpacity = particleOpacity;
	if (particleUV.x > LINE_WIDTH && particleUV.x < 1.0 - LINE_WIDTH && particleUV.y > LINE_WIDTH && particleUV.y < 1.0 - LINE_WIDTH)
		finalOpacity = 0.0;
	OUT_COLOR = vec4(vec3(1.0 - finalOpacity), 1.0);
}