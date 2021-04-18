#include "Defaults.h"

sf::Shader sf::Defaults::shader;
sf::Material sf::Defaults::material;

void sf::Defaults::Initialize()
{
	shader.CreateFromFiles("assets/shaders/defaultV.shader", "assets/shaders/defaultF.shader");
	material.CreateFromShader(&shader, true);
}
