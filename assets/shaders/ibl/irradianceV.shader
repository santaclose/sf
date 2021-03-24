#version 330 core

layout(location = 0) in vec3 position;

out vec3 cubeCoords;

uniform mat4 projection;
uniform mat4 view;


void main()
{
    cubeCoords = position;

    gl_Position = projection * view * vec4(cubeCoords, 1.0f);
}
