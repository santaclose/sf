#pragma once

#include "Texture.h"
#include "Cubemap.h"

namespace IblTools {

	void HdrToCubemaps(const Texture& hdrTexture, Cubemap& environmentCubemap, Cubemap& diffuseIrradiance, Cubemap& prefiltered);
}