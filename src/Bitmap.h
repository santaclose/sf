#pragma once

#include <string>
#include <DataTypes.h>

namespace sf
{
	struct Bitmap
	{
		DataType dataType = DataType::u8;
		uint8_t channelCount = 3;
		uint32_t width = 0;
		uint32_t height = 0;
		void* buffer = nullptr;

		Bitmap::Bitmap() = default;
		Bitmap::Bitmap(DataType dataType, uint8_t channelCount, uint32_t width, uint32_t height, const void* pixelValue = nullptr);
		Bitmap::Bitmap(const std::string& filePath, bool flipVertically = true);
		void AddChannels(uint8_t channelCount = 1);
		void CopyChannel(const Bitmap& source, uint8_t sourceChannel, uint8_t targetChannel);
		void WritePng(const std::string& filePath);
		void WritePpm(const std::string& filePath);
		Bitmap::~Bitmap();
	};
}