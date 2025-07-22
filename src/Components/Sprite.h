#pragma once

#include <Bitmap.h>
#include <Alignment.h>

namespace sf {

	struct Sprite
	{
		const Bitmap* bitmap;
		int alignmentH = ALIGNMENT_LEFT;
		int alignmentV = ALIGNMENT_TOP;

		inline Sprite(const Bitmap* bitmap)
		{
			this->bitmap = bitmap;
		}
	};
}