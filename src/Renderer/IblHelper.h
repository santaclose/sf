#pragma once

#include <Renderer/GlTexture.h>
#include <Renderer/GlCubemap.h>

namespace sf::IblHelper
{
	void GenerateLUT(GlTexture& lut);
	void CubemapFromHdr(const std::string &hdrFilePath, GlCubemap& environmentCubemap);
	void IrradianceFromEnv(const GlCubemap& environmentCubemap, GlCubemap& irradianceCubemap);
	void SpecularFromEnv(const GlCubemap& environmentCubemap, GlCubemap& prefilterCubemap);
}