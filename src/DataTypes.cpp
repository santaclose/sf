#include "DataTypes.h"

namespace sf {

	std::unordered_map<DataType, unsigned int> dataTypeSizes = {
		{DataType::b, 1},
		{DataType::i8, 1},
		{DataType::i16, 2},
		{DataType::i32, 4},
		{DataType::i64, 8},
		{DataType::u8, 1},
		{DataType::u16, 2},
		{DataType::u32, 4},
		{DataType::u64, 8},
		{DataType::f16, 2},
		{DataType::f32, 4},
		{DataType::f64, 8},
		{DataType::vec2f32, 8},
		{DataType::vec3f32, 12},
		{DataType::vec4f32, 16},
		{DataType::mat2f32, 16},
		{DataType::mat3f32, 36},
		{DataType::mat4f32, 64},
		{DataType::vec2f64, 16},
		{DataType::vec3f64, 24},
		{DataType::vec4f64, 32},
		{DataType::mat2f64, 32},
		{DataType::mat3f64, 72},
		{DataType::mat4f64, 128}
	};

	unsigned int sf::GetDataTypeSize(DataType dataType)
	{
		return dataTypeSizes[dataType];
	}
}