#pragma once

#include <Renderer/GlTexture.h>
#include <Renderer/GlCubemap.h>

namespace sf::IblHelper
{
	void GenerateLUT(GlTexture& lut, DataType dataType = DataType::f16);
	void CubemapFromHdr(const std::string &hdrFilePath, GlCubemap& environmentCubemap, DataType dataType = DataType::f16);
	void SpecularFromEnv(const GlCubemap& environmentCubemap, GlCubemap& prefilterCubemap, DataType dataType = DataType::f16);
	void IrradianceFromEnv(const GlCubemap& environmentCubemap, GlCubemap& irradianceCubemap, DataType dataType = DataType::f16);
}