
out vec2 fTexCoords;

void main()
{
	fTexCoords = VA_TEX_COORDS;
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * PARTICLE_MATRICES[gl_InstanceID] * vec4(VA_POSITION, 1.0);
}