#include "DataTypes.h"

namespace sf {

	unsigned int sf::GetDataTypeSize(DataType dataType)
	{
		switch (dataType)
		{
		case DataType::b: return 1;
		case DataType::i8: return 1;
		case DataType::i16: return 2;
		case DataType::i32: return 4;
		case DataType::i64: return 8;
		case DataType::u8: return 1;
		case DataType::u16: return 2;
		case DataType::u32: return 4;
		case DataType::u64: return 8;
		case DataType::f16: return 2;
		case DataType::f32: return 4;
		case DataType::f64: return 8;
		case DataType::vec2f32: return 8;
		case DataType::vec3f32: return 12;
		case DataType::vec4f32: return 16;
		case DataType::mat2f32: return 16;
		case DataType::mat3f32: return 36;
		case DataType::mat4f32: return 64;
		case DataType::vec2f64: return 16;
		case DataType::vec3f64: return 24;
		case DataType::vec4f64: return 32;
		case DataType::mat2f64: return 32;
		case DataType::mat3f64: return 72;
		case DataType::mat4f64: return 128;
		}
	}
}