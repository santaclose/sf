#pragma once

#include <Bitmap.h>

namespace sf {

	struct Sprite
	{
	private:
		static int counter;

	public:
		int id;
		const Bitmap* bitmap;

		Sprite(const Bitmap* bitmap);
	};
}