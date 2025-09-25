#include "Bitmap.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <assert.h>
#include <stb_image.h>
#include <stb_image_write.h>

sf::Bitmap::Bitmap(DataType dataType, uint8_t channelCount, uint32_t width, uint32_t height, const void* pixelValue)
{
	uint32_t dataTypeSize = GetDataTypeSize(dataType);

	this->dataType = dataType;
	this->channelCount = channelCount;
	this->width = width;
	this->height = height;
	this->buffer = malloc(dataTypeSize * (this->width) * (this->height) * (this->channelCount));

	if (pixelValue == nullptr)
		return; // leave uninitialized

	assert(sizeof(uint8_t) == 1);
	for (int i = 0; i < (this->width) * (this->height); i++)
	{
		memcpy(((uint8_t*)this->buffer) + (i * dataTypeSize * this->channelCount), pixelValue, dataTypeSize * (this->channelCount));
	}
}

sf::Bitmap::Bitmap(const std::string& filePath, bool flipVertically, bool limitRangeTo16bitFloat)
{
	assert(GetDataTypeSize(DataType::f32) == sizeof(float));
	assert(GetDataTypeSize(DataType::u8) == sizeof(stbi_uc));

	void* stb_buffer;
	stbi_set_flip_vertically_on_load(flipVertically);
	std::string fileExtension = filePath.substr(filePath.find_last_of('.') + 1);
	int x, y, c;
	if (fileExtension == "hdr")
	{
		stb_buffer = stbi_loadf(filePath.c_str(), &x, &y, &c, 0);
		this->dataType = DataType::f32;
	}
	else
	{
		stb_buffer = stbi_load(filePath.c_str(), &x, &y, &c, 0);
		this->dataType = DataType::u8;
	}
	if (!stb_buffer)
	{
		std::cout << "[Bitmap] Failed to load file: " << filePath << std::endl;
		return;
	}

	this->width = x;
	this->height = y;
	this->channelCount = c;

	uint32_t dataTypeSize = GetDataTypeSize(this->dataType);
	this->buffer = malloc(dataTypeSize * (this->width) * (this->height) * (this->channelCount));
	memcpy(this->buffer, stb_buffer, this->width * this->height * this->channelCount * dataTypeSize);

	stbi_image_free(stb_buffer);

	if (limitRangeTo16bitFloat && this->dataType == DataType::f32)
	{
		uint32_t dataTypeSize = GetDataTypeSize(this->dataType);
		for (int i = 0; i < (this->width) * (this->height); i++)
		{
			for (int j = 0; j < this->channelCount; j++)
			{
				float* pointer = (float*)(((uint8_t*)this->buffer) + (i * dataTypeSize * (this->channelCount)) + (dataTypeSize * j));
				if (*pointer > 6.55e4)
					*pointer = 6.55e4;
			}
		}
	}
}

void sf::Bitmap::AddChannels(uint8_t channelCount)
{
	void* oldBuffer = this->buffer;

	uint8_t originalChannelCount = this->channelCount;

	this->channelCount += channelCount;
	this->buffer = malloc(GetDataTypeSize(this->dataType) * (this->width) * (this->height) * (this->channelCount));

	uint32_t dataTypeSize = GetDataTypeSize(this->dataType);

	if (this->channelCount == 1)
		return; // nothing to copy 

	assert(sizeof(uint8_t) == 1);
	for (int i = 0; i < (this->width) * (this->height); i++)
	{
		memcpy(
			((uint8_t*)this->buffer) + (i * dataTypeSize * (this->channelCount)),
			((uint8_t*)oldBuffer) + (i * dataTypeSize * originalChannelCount),
			dataTypeSize * originalChannelCount);
	}

	free(oldBuffer);
}

void sf::Bitmap::CopyChannel(const Bitmap& source, uint8_t sourceChannel, uint8_t targetChannel)
{
	assert(source.dataType == this->dataType);
	assert(source.width == this->width);
	assert(source.height == this->height);

	uint32_t dataTypeSize = GetDataTypeSize(this->dataType);

	assert(sizeof(uint8_t) == 1);
	for (int i = 0; i < (this->width) * (this->height); i++)
	{
		memcpy(
			((uint8_t*)this->buffer) + (i * dataTypeSize * (this->channelCount)) + (dataTypeSize * targetChannel),
			((uint8_t*)source.buffer) + (i * dataTypeSize * (source.channelCount)) + (dataTypeSize * sourceChannel),
			dataTypeSize);
	}
}

void sf::Bitmap::WritePng(const std::string& filePath)
{
	assert(this->dataType == DataType::u8);
	assert(this->channelCount < 5 && this->channelCount > 0);
	stbi_write_png(filePath.c_str(), this->width, this->height, this->channelCount, this->buffer, (this->width) * (this->channelCount));
}

void sf::Bitmap::WritePpm(const std::string& filePath)
{
	assert(this->dataType == DataType::u8);
	assert(this->channelCount == 3);

	uint32_t dataTypeSize = GetDataTypeSize(this->dataType);

	std::ofstream outputFile;
	outputFile.open(filePath);

	outputFile << "P3\n" << this->width << ' ' << this->height << "\n255\n";

	assert(sizeof(uint8_t) == 1);
	for (int i = 0; i < this->width; i++)
	{
		for (int j = 0; j < this->height; j++)
		{
			for (int k = 0; k < this->channelCount; k++)
			{
				uint8_t* component = ((uint8_t*)this->buffer) + (i * dataTypeSize * (this->channelCount) * (this->width)) + (j * dataTypeSize * (this->channelCount)) + k;
				if (k > 0)
					outputFile << ' ';
				outputFile << (int)*component;
			}
			outputFile << '\n';
		}
	}
}

sf::Bitmap::~Bitmap()
{
	free(this->buffer);
}
