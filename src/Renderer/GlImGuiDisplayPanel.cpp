#include "ImGuiDisplayPanel.h"

#include "GlFramebuffer.h"
#include <glad/glad.h>
#include <iostream>
#include <cstdint>

bool operator!=(const ImVec2& a, const ImVec2& b)
{
	return a.x != b.x || a.y != b.y;
}
ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x + b.x, a.y + b.y);
}
ImVec2 operator-(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x - b.x, a.y - b.y);
}

void sf::ImGuiDisplayPanel::Create(const char* name, void (*drawFunction)(uint32_t, float), uint32_t samples, bool includeDepth)
{
	assert(drawFunction != nullptr);
	GlFramebuffer* fbmsaa = nullptr;
	GlFramebuffer* fb = nullptr;
	m_name = name;
	m_drawFunction = drawFunction;
	m_includeDepth = includeDepth;
	m_samples = samples;
	assert(m_samples == 1 || m_samples == 2 || m_samples == 4 || m_samples == 8);
	if (m_samples > 1)
	{
		fbmsaa = new GlFramebuffer();
		m_gl_framebufferMSAA = (void*) fbmsaa;
		fbmsaa->Create(0, 0, m_samples, m_includeDepth);

		m_framebufferMSAA.id = &fbmsaa->m_gl_id;
		m_framebufferMSAA.size = &m_size;
		m_framebufferMSAA.hasDepth = m_includeDepth;
	}
	fb = new GlFramebuffer();
	m_gl_framebuffer = (void*) fb;
	fb->Create(0, 0, 1, m_includeDepth);

	m_framebuffer.id = &fb->m_gl_id;
	m_framebuffer.size = &m_size;
	m_framebuffer.hasDepth = m_includeDepth;
}

void sf::ImGuiDisplayPanel::Delete()
{
	GlFramebuffer* fbmsaa = (GlFramebuffer*) m_gl_framebufferMSAA;
	GlFramebuffer* fb = (GlFramebuffer*) m_gl_framebuffer;
	delete fb;
	if (fbmsaa != nullptr)
		delete fbmsaa;
}

void sf::ImGuiDisplayPanel::ImGuiCall(const ImGuiIO& io, bool& cursorHover, uint32_t panelId, float deltaTime)
{
	GlFramebuffer* fbmsaa = (GlFramebuffer*) m_gl_framebufferMSAA;
	GlFramebuffer* fb = (GlFramebuffer*) m_gl_framebuffer;

	cursorHover = false;
	ImGui::Begin(m_name);

	if (ImGui::IsMouseHoveringRect(
			ImGui::GetWindowContentRegionMin() + ImGui::GetWindowPos(),
			ImGui::GetWindowContentRegionMax() + ImGui::GetWindowPos()))
	{
		ImVec2 mousePos = io.MousePos - ImGui::GetWindowContentRegionMin() - ImGui::GetWindowPos();
		glm::vec2 mp = glm::vec2(mousePos.x, mousePos.y);
		cursorHover = true;
	}
	ImVec2 avail = ImGui::GetContentRegionAvail();
	glm::uvec2 currentSize(
		std::max(0, (int)avail.x),
		std::max(0, (int)avail.y)
	);
	if (currentSize != m_size)
	{
		m_size = currentSize;
		if (m_samples > 1)
			fbmsaa->Resize(m_size.x, m_size.y);
		fb->Resize(m_size.x, m_size.y);
	}

	m_drawFunction(panelId, deltaTime);
	if (m_samples > 1)
	{
		fbmsaa->Unbind();

		fb->Bind(GL_DRAW_FRAMEBUFFER);
		fbmsaa->Bind(GL_READ_FRAMEBUFFER);
		glBlitFramebuffer(0, 0, m_size.x, m_size.y, 0, 0, m_size.x, m_size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		fbmsaa->Unbind(GL_READ_FRAMEBUFFER);
		fb->Unbind(GL_DRAW_FRAMEBUFFER);
	}
	else
		fb->Unbind();
	uint32_t textureID = fb->getColorAttachmentID();
	ImVec2 sizeFloats = { (float) m_size.x, (float) m_size.y };
	ImGui::Image((void*)textureID, sizeFloats, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();
}

void sf::ImGuiDisplayPanel::SetSamples(uint32_t samples)
{
	GlFramebuffer* fbmsaa = (GlFramebuffer*) m_gl_framebufferMSAA;

	m_samples = samples;
	assert(m_samples == 1 || m_samples == 2 || m_samples == 4 || m_samples == 8);
	if (m_samples > 1)
	{
		if (fbmsaa == nullptr)
		{
			fbmsaa = new GlFramebuffer();
			m_gl_framebufferMSAA = (void*) fbmsaa;
			fbmsaa->Create(0, 0, m_samples, m_includeDepth);

			m_framebufferMSAA.id = &fbmsaa->m_gl_id;
			m_framebufferMSAA.size = &m_size;
			m_framebufferMSAA.hasDepth = m_includeDepth;
		}
		fbmsaa->SetSamples(m_samples);
	}
}