#pragma once

#include <Renderer/Texture.h>
#include <Renderer/Cubemap.h>

namespace sf::IblHelper {

	void HdrToCubemaps(const Texture& hdrTexture, Cubemap& environmentCubemap, Cubemap& irradianceCubemap, Cubemap& prefilterCubemap, Texture& lookupTexture);
}