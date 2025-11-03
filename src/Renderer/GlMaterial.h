#pragma once

#include <vector>
#include <string>

#include <Material.h>

#include <Renderer/GlShader.h>
#include <Renderer/GlTexture.h>

namespace sf {

	struct GlMaterial
	{
		GlShader* m_shader;
	private:
		const Material* m_material;
		std::unordered_map<void*, void*> m_textures;
		std::unordered_map<void*, uint32_t> m_ssbos;

	public:
		void Create(const Material* material, const BufferLayout* vertexBufferLayout);
		void Bind(const std::vector<void*>& rendererUniformVector);
		void UpdateBufferData(uint32_t bufferIndex, uint32_t location = ~0, uint32_t size = ~0);
	};
}