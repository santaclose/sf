#pragma once

#include <unordered_map>

namespace sf
{
	enum class DataType {
		b = 0,
		i8 = 1,
		i16 = 2,
		i32 = 3,
		i64 = 4,
		u8 = 5,
		u16 = 6,
		u32 = 7,
		u64 = 8,
		f16 = 9,
		f32 = 10,
		f64 = 11,

		vec2i8 = 12,
		vec3i8 = 13,
		vec4i8 = 14,
		mat2i8 = 15,
		mat3i8 = 16,
		mat4i8 = 17,

		vec2i16 = 18,
		vec3i16 = 19,
		vec4i16 = 20,
		mat2i16 = 21,
		mat3i16 = 22,
		mat4i16 = 23,

		vec2i32 = 24,
		vec3i32 = 25,
		vec4i32 = 26,
		mat2i32 = 27,
		mat3i32 = 28,
		mat4i32 = 29,

		vec2i64 = 30,
		vec3i64 = 31,
		vec4i64 = 32,
		mat2i64 = 33,
		mat3i64 = 34,
		mat4i64 = 35,

		vec2u8 = 36,
		vec3u8 = 37,
		vec4u8 = 38,
		mat2u8 = 39,
		mat3u8 = 40,
		mat4u8 = 41,

		vec2u16 = 42,
		vec3u16 = 43,
		vec4u16 = 44,
		mat2u16 = 45,
		mat3u16 = 46,
		mat4u16 = 47,

		vec2u32 = 48,
		vec3u32 = 49,
		vec4u32 = 50,
		mat2u32 = 51,
		mat3u32 = 52,
		mat4u32 = 53,

		vec2u64 = 54,
		vec3u64 = 55,
		vec4u64 = 56,
		mat2u64 = 57,
		mat3u64 = 58,
		mat4u64 = 59,

		vec2f16 = 60,
		vec3f16 = 61,
		vec4f16 = 62,
		mat2f16 = 63,
		mat3f16 = 64,
		mat4f16 = 65,

		vec2f32 = 66,
		vec3f32 = 67,
		vec4f32 = 68,
		mat2f32 = 69,
		mat3f32 = 70,
		mat4f32 = 71,

		vec2f64 = 72,
		vec3f64 = 73,
		vec4f64 = 74,
		mat2f64 = 75,
		mat3f64 = 76,
		mat4f64 = 77
	};

	enum class ShaderDataType {
		bitmap = 78,
		cubemap = 79
	};

	uint32_t GetDataTypeSize(DataType dataType);
}