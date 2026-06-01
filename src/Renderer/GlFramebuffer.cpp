#include "GlFramebuffer.h"
#include <iostream>

void GlFramebuffer::Create(uint32_t width, uint32_t height, uint32_t samples, bool includeDepth)
{
	m_width = width;
	m_height = height;
	m_samples = samples;
	m_includeDepth = includeDepth;
	if (m_width > 0 && m_height > 0)
		Invalidate();
}

void GlFramebuffer::Delete()
{
	if (m_gl_id != 0)
	{
		glDeleteFramebuffers(1, &m_gl_id);
		glDeleteTextures(1, &m_gl_color_attachment);
		if (m_includeDepth)
		{
			if (m_samples == 1)
				glDeleteRenderbuffers(1, &m_gl_depth_attachment);
			else
				glDeleteTextures(1, &m_gl_depth_attachment);
		}
	}
	m_gl_id = 0;
	m_gl_color_attachment = 0;
	m_gl_depth_attachment = 0;
}

void GlFramebuffer::Invalidate()
{
	Delete();

	glCreateFramebuffers(1, &m_gl_id);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gl_id);

	if (m_samples == 1)
		glCreateTextures(GL_TEXTURE_2D, 1, &m_gl_color_attachment);
	else
		glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_gl_color_attachment);
	if (m_includeDepth)
	{
		if (m_samples == 1)
			glGenRenderbuffers(1, &m_gl_depth_attachment);
		else
			glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_gl_depth_attachment);
	}

	if (m_samples == 1)
	{
		glBindTexture(GL_TEXTURE_2D, m_gl_color_attachment);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gl_color_attachment, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_gl_color_attachment);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_RGB, m_width, m_height, GL_TRUE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_gl_color_attachment, 0);
	}

	if (m_includeDepth)
	{
		if (m_samples == 1)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, m_gl_depth_attachment);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_gl_depth_attachment);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_gl_depth_attachment);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_samples, GL_DEPTH_COMPONENT, m_width, m_height, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, m_gl_depth_attachment, 0);
		}
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "[GlFramebuffer] Error creating multisample framebuffer with width " << m_width << " and height " << m_height << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GlFramebuffer::Resize(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0)
		return;
	m_width = width;
	m_height = height;
	Invalidate();
}

void GlFramebuffer::SetSamples(uint32_t samples)
{
	m_samples = samples;
	assert(m_samples == 1 || m_samples == 2 || m_samples == 4 || m_samples == 8);
	if (m_width > 0 && m_height > 0)
		Invalidate();
}

void GlFramebuffer::Bind(GLenum target)
{
	assert(target == GL_READ_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER);
	glBindFramebuffer(target, m_gl_id);
	if (target != GL_READ_FRAMEBUFFER)
		glViewport(0, 0, m_width, m_height);
}

void GlFramebuffer::Unbind(GLenum target)
{
	assert(target == GL_READ_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER);
	glBindFramebuffer(target, 0);
}

float GlFramebuffer::GetAspectRatio()
{
	return (float) m_width / (float) m_height;
}

uint32_t GlFramebuffer::getColorAttachmentID()
{
	return m_gl_color_attachment;
}
