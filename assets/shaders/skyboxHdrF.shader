#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform bool useExposure = false;
uniform float exposure = 5.0;

const float gamma = 2.2;

void main()
{
    if (useExposure)
    {
        vec3 hdrColor = texture(skybox, TexCoords).rgb;

        // exposure tone mapping
        vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
        // gamma correction 
        mapped = pow(mapped, vec3(1.0 / gamma));

        FragColor = vec4(mapped, 1.0);
    }
    else
    {
        vec3 envColor = textureLod(skybox, TexCoords, 0.0).rgb;

        // HDR tonemap and gamma correct
        envColor = envColor / (envColor + vec3(1.0));
        envColor = pow(envColor, vec3(1.0 / 2.2));

        FragColor = vec4(envColor, 1.0);
    }
}
