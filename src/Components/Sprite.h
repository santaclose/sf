#pragma once

#include <Bitmap.h>
#include <Alignment.h>

namespace sf {

	struct Sprite
	{
	private:
		static int counter;

	public:
		int id;
		const Bitmap* bitmap;
		int alignmentH = ALIGNMENT_LEFT;
		int alignmentV = ALIGNMENT_TOP;

		Sprite(const Bitmap* bitmap);
	};
}