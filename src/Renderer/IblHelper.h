#pragma once

#include <Renderer/GlTexture.h>
#include <Renderer/GlCubemap.h>

namespace sf::IblHelper {

	void HdrToCubemaps(const GlTexture& hdrTexture, GlCubemap& environmentCubemap, GlCubemap& irradianceCubemap, GlCubemap& prefilterCubemap, GlTexture& lookupTexture);
}