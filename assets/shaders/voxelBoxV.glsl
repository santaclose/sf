
out vec2 fTexCoords;

void main()
{
	fTexCoords = VA_TEX_COORDS;
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * PARTICLE_MATRICES[INSTANCE_INDEX] * vec4(VA_POSITION, 1.0);
}