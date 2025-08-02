layout(location = 0) in vec2 fScreenPos;

#include <assets/shaders/random.h>


void main()
{
	OUT_COLOR = vec4(vec3(Random(fScreenPos)), 1.0);
}