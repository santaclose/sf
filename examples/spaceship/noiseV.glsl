
out vec2 fScreenPos;

void main()
{
	gl_Position = CAMERA_MATRIX * OBJECT_MATRIX * vec4(VA_POSITION, 1.0);
	fScreenPos = gl_Position.xy;
}