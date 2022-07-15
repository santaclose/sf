#pragma once

#include <vector>
#include <string>

#include <Material.h>

#include <Renderer/GlShader.h>
#include <Renderer/GlTexture.h>

namespace sf {

	class GlMaterial
	{
	public:
		enum UniformType {
			_1i,
			_2f,
			_3f,
			_4f,
			_Texture,
			_Cubemap
		};

		GlShader* m_shader;
	private:
		std::vector<std::string> m_uniformNames;
		std::vector<void*> m_uniformData;
		std::vector<UniformType> m_uniformTypes;
		bool m_isDoubleSided;

	public:
		void Create(const Material& material, const std::vector<void*>& rendererUniformVector);
		void CreateFromShader(GlShader* theShader, bool isDoubleSided = false);
		void SetUniform(const std::string& name, void* data, UniformType type);
		void Bind();
	};
}