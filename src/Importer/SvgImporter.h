#pragma once

#include <vector>
#include <string>

#include <Bitmap.h>

namespace sf::SvgImporter
{
	int Load(const std::string& filePath, const char* units = "px", float dpi = 96.0f);
	void Destroy(int id);
	void RenderToBitmap(int id, Bitmap& bitmap, float scale = 1.0f);
}