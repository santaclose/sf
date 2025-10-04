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

	public:
		void Create(const Material* material,
			const BufferLayout& vertexBufferLayout,
			const BufferLayout* voxelBufferLayout = nullptr,
			const BufferLayout* particleBufferLayout = nullptr);
		void Bind(const std::vector<void*>& rendererUniformVector);
	};
}