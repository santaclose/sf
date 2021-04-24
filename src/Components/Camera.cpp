#include "Camera.h"

#include <Renderer/Renderer.h>

int sf::Camera::counter = 0;

sf::Camera::Camera()
{
	id = counter;
	counter++;
}