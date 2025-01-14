#include "DataTypes.h"
#include <cassert>

namespace sf {

	uint32_t GetDataTypeSize(DataType dataType)
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

		case DataType::vec2i8: return 2;
		case DataType::vec3i8: return 3;
		case DataType::vec4i8: return 4;
		case DataType::mat2i8: return 4;
		case DataType::mat3i8: return 9;
		case DataType::mat4i8: return 16;

		case DataType::vec2i16: return 4;
		case DataType::vec3i16: return 6;
		case DataType::vec4i16: return 8;
		case DataType::mat2i16: return 8;
		case DataType::mat3i16: return 18;
		case DataType::mat4i16: return 32;

		case DataType::vec2i32: return 8;
		case DataType::vec3i32: return 12;
		case DataType::vec4i32: return 16;
		case DataType::mat2i32: return 16;
		case DataType::mat3i32: return 36;
		case DataType::mat4i32: return 64;

		case DataType::vec2i64: return 16;
		case DataType::vec3i64: return 24;
		case DataType::vec4i64: return 32;
		case DataType::mat2i64: return 32;
		case DataType::mat3i64: return 72;
		case DataType::mat4i64: return 128;

		case DataType::vec2u8: return 2;
		case DataType::vec3u8: return 3;
		case DataType::vec4u8: return 4;
		case DataType::mat2u8: return 4;
		case DataType::mat3u8: return 9;
		case DataType::mat4u8: return 16;

		case DataType::vec2u16: return 4;
		case DataType::vec3u16: return 6;
		case DataType::vec4u16: return 8;
		case DataType::mat2u16: return 8;
		case DataType::mat3u16: return 18;
		case DataType::mat4u16: return 32;

		case DataType::vec2u32: return 8;
		case DataType::vec3u32: return 12;
		case DataType::vec4u32: return 16;
		case DataType::mat2u32: return 16;
		case DataType::mat3u32: return 36;
		case DataType::mat4u32: return 64;

		case DataType::vec2u64: return 16;
		case DataType::vec3u64: return 24;
		case DataType::vec4u64: return 32;
		case DataType::mat2u64: return 32;
		case DataType::mat3u64: return 72;
		case DataType::mat4u64: return 128;

		case DataType::vec2f16: return 4;
		case DataType::vec3f16: return 6;
		case DataType::vec4f16: return 8;
		case DataType::mat2f16: return 8;
		case DataType::mat3f16: return 18;
		case DataType::mat4f16: return 32;

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
		default: assert(false); return ~0;
		}
	}
}