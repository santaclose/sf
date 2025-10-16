#pragma once

#include <string>
#include <glm/glm.hpp>
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

		Bitmap() = default;
		void CreateSolid(DataType dataType, uint8_t channelCount, uint32_t width, uint32_t height, const void* pixelValue = nullptr);
		void CreateFromFile(const std::string& filePath, bool flipVertically = true, bool limitRangeTo16bitFloat = false);
		void AddChannels(uint8_t channelCount = 1);
		void CopyChannel(const Bitmap& source, uint8_t sourceChannel, uint8_t targetChannel);
		void WritePng(const std::string& filePath);
		void WritePpm(const std::string& filePath);
		template <typename T>
		inline float Sample(const glm::vec2& uv, uint8_t channel) const
		{
			uint32_t dataTypeSize = GetDataTypeSize(this->dataType);
			/* Adjust so 0,0 lands on corner of pixel and not its center */
			float uAdjustedCorners = uv.x * (float)width - 0.5f;
			float vAdjustedCorners = uv.y * (float)height - 0.5f;
			uint32_t targetX = (uint32_t) uAdjustedCorners;
			uint32_t targetY = (uint32_t) vAdjustedCorners;
			float uFraction = uAdjustedCorners - (float) targetX;
			float vFraction = vAdjustedCorners - (float) targetY;
			T* a = (T*)(((uint8_t*)this->buffer) +
				(targetY * dataTypeSize * this->channelCount * this->width) +
				(targetX * dataTypeSize * this->channelCount) + channel);
			T* b = (T*)(((uint8_t*)this->buffer) +
				(targetY * dataTypeSize * this->channelCount * this->width) +
				(glm::min(targetX + 1, width - 1) * dataTypeSize * this->channelCount) + channel);
			T* c = (T*)(((uint8_t*)this->buffer) +
				(glm::min(targetY + 1, height - 1) * dataTypeSize * this->channelCount * this->width) +
				(targetX * dataTypeSize * this->channelCount) + channel);
			T* d = (T*)(((uint8_t*)this->buffer) +
				(glm::min(targetY + 1, height - 1) * dataTypeSize * this->channelCount * this->width) +
				(glm::min(targetX + 1, width - 1) * dataTypeSize * this->channelCount) + channel);
			float af = (float)(*a) / (float)((1 << (sizeof(T) * 8)) - 1);
			float bf = (float)(*b) / (float)((1 << (sizeof(T) * 8)) - 1);
			float cf = (float)(*c) / (float)((1 << (sizeof(T) * 8)) - 1);
			float df = (float)(*d) / (float)((1 << (sizeof(T) * 8)) - 1);
			return glm::mix(glm::mix(af, bf, uFraction), glm::mix(cf, df, uFraction), vFraction);
		}
		~Bitmap();
	};
}