#pragma once
#include <imgui.h>
#include <glm/glm.hpp>

#include <Renderer/RenderTarget.h>
namespace sf
{
	class ImGuiDisplayPanel
	{
	private:
		const char* m_name;
		glm::uvec2 m_size = { 0, 0 };
		void* m_gl_framebuffer = nullptr;
		void* m_gl_framebufferMSAA = nullptr;
		bool m_includeDepth = false;
		uint32_t m_samples = 0;
		void (*m_drawFunction)(uint32_t, float);
		Renderer::Framebuffer m_framebuffer;
		Renderer::Framebuffer m_framebufferMSAA;

	public:
		void Create(const char* name, void (*drawFunction)(uint32_t, float), uint32_t samples = 4, bool includeDepth = true);
		void Delete();
		void ImGuiCall(const ImGuiIO& io, bool& cursorHover, uint32_t panelId, float deltaTime);
		void SetSamples(uint32_t samples);
		Renderer::Framebuffer GetFramebufferToDraw() const { return m_samples > 1 ? m_framebufferMSAA : m_framebuffer; }
	};
}