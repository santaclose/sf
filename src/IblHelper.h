#pragma once

#include <Texture.h>
#include <Cubemap.h>

namespace sf::IblHelper {

	void HdrToCubemaps(const Texture& hdrTexture, Cubemap& environmentCubemap, Cubemap& irradianceCubemap, Cubemap& prefilterCubemap, Texture& lookupTexture);
}