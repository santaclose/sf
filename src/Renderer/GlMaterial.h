#pragma once

#include <vector>
#include <string>

#include <Renderer/GlShader.h>
#include <Renderer/GlTexture.h>

namespace sf {

	class GlMaterial
	{
	public:
		enum UniformType {
			_Boolean,
			_Texture,
			_Cubemap,
			_Color
		};

		GlShader* m_shader;
	private:
		std::vector<std::string> m_uniformNames;
		std::vector<void*> m_uniformData;
		std::vector<UniformType> m_uniformTypes;
		bool m_isDoubleSided;

	public:
		void CreateFromShader(GlShader* theShader, bool isDoubleSided = false);
		void SetUniform(const std::string& name, void* data, UniformType type);
		void Bind();
	};
}