#include "Sprite.h"

int sf::Sprite::counter = 0;

sf::Sprite::Sprite(const Bitmap* bitmap)
{
	this->bitmap = bitmap;
	this->id = counter;
	counter++;
}
