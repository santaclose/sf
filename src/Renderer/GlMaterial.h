#pragma once

#include <vector>
#include <string>

#include <Material.h>

#include <Renderer/GlShader.h>
#include <Renderer/GlTexture.h>

namespace sf {

	struct GlMaterial
	{
		GlShader* m_shader = nullptr;
	private:
		const Material* m_material = nullptr;
		std::unordered_map<const void*, GlTexture*> m_textures;
		std::unordered_map<const void*, uint32_t> m_ssbos;

	public:
		void Create(const Material* material, Buffer::Layout vertexBufferLayout);
		void Delete();
		void Bind(const std::vector<void*>& rendererUniformVector, const std::unordered_map<std::string, UniformData>* uniformOverrides = nullptr);
		void UpdateBufferData(uint32_t bufferIndex, uint32_t location = ~0, uint32_t size = ~0);
	};
}