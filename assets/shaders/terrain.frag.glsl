in TE_OUT {
	vec3 worldPos;
	vec2 uv;
} fs_in;

uniform sampler2D heightmapTexture;

void main()
{
	OUT_COLOR = vec4(vec3(texture(heightmapTexture, fs_in.uv).r), 1.0);
}