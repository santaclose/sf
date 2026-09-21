#include "SvgImporter.h"

#include <nanosvg.h>
#include <nanosvgrast.h>

namespace sf::SvgImporter {

	std::vector<NSVGimage*> svgData;
}

int sf::SvgImporter::Load(const std::string& filePath, const char* units, float dpi)
{
	static std::unordered_map <std::string, int> alreadyImportedCache;
	if (alreadyImportedCache.find(filePath) != alreadyImportedCache.end())
		return alreadyImportedCache[filePath];

	NSVGimage* image = nsvgParseFromFile(filePath.c_str(), units, dpi);
	if (image == nullptr)
	{
		printf("[SvgImporter] Could not read file: %s\n", filePath.c_str());
		return -1;
	}

	int newId = svgData.size();
	alreadyImportedCache[filePath] = newId;

	printf("[SvgImporter] Assigning ID %u to file %s\n", newId, filePath.c_str());
	svgData.push_back(image);
	return newId;
}

void sf::SvgImporter::Destroy(int id)
{
	assert(id > -1 && id < svgData.size());
	assert(svgData[id] != nullptr);
	nsvgDelete(svgData[id]);
	svgData[id] = nullptr;
}

bool sf::SvgImporter::RenderToBitmap(int id, Bitmap& bitmap, float scale)
{
	assert(id > -1 && id < svgData.size());
	assert(svgData[id] != nullptr);

	NSVGimage* image = svgData[id];
	NSVGrasterizer* rast = NULL;

	int w = (int)(image->width * scale) + 1;
	int h = (int)(image->height * scale) + 1;

	rast = nsvgCreateRasterizer();
	if (rast == NULL)
	{
		printf("[SvgImporter] Could not init rasterizer.\n");
		return false;
	}

	bitmap.Destroy();

	bitmap.dataType = DataType::u8;
	bitmap.channelCount = 4;
	bitmap.width = w;
	bitmap.height = h;
	bitmap.buffer = new uint8_t[w * h * 4];

	nsvgRasterize(rast, image, 0, 0, scale, (unsigned char *)bitmap.buffer, w, h, w * 4);

	nsvgDeleteRasterizer(rast);
	return true;
}
