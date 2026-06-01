#pragma once

#include <glad/glad.h>
#include <imgui.h>
#include <cstdint>

struct GlFramebuffer
{
	uint32_t m_gl_id = 0;
	uint32_t m_gl_color_attachment = 0, m_gl_depth_attachment = 0;
	uint32_t m_width = 0, m_height = 0;
	uint32_t m_samples = 4;
	bool m_includeDepth = false;

	void Create(uint32_t width, uint32_t height, uint32_t samples = 4, bool includeDepth = false);
	void Delete();

	void Invalidate();
	void Resize(uint32_t width, uint32_t height);
	void SetSamples(uint32_t samples);
	void Bind(GLenum target = GL_FRAMEBUFFER);
	void Unbind(GLenum target = GL_FRAMEBUFFER);
	float GetAspectRatio();
	uint32_t getColorAttachmentID();
};