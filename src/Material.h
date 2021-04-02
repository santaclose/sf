#pragma once

#include <vector>
#include <string>
#include "Shader.h"
#include "Texture.h"

namespace sf {

	class Material
	{
	public:
		enum UniformType {
			_Boolean,
			_Texture,
			_Cubemap,
			_Color
		};

		Shader* m_shader;
	private:
		std::vector<std::string> m_uniformNames;
		std::vector<void*> m_uniformData;
		std::vector<UniformType> m_uniformTypes;

	public:
		void CreateFromShader(Shader* theShader);
		void SetUniform(const std::string& name, void* data, UniformType type);
		void Bind();

		static Material* boundMaterial;
	};
}