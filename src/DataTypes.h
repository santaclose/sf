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
		vec2f32 = 12,
		vec3f32 = 13,
		vec4f32 = 14,
		mat2f32 = 15,
		mat3f32 = 16,
		mat4f32 = 17,
		vec2f64 = 18,
		vec3f64 = 19,
		vec4f64 = 20,
		mat2f64 = 21,
		mat3f64 = 22,
		mat4f64 = 23
	};

	enum class ShaderDataType {
		bitmap = 24,
		cubemap = 25
	};

	uint32_t GetDataTypeSize(DataType dataType);
}