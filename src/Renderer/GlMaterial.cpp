#include "GlMaterial.h"

#include <iostream>

#include <Renderer/GlTexture.h>
#include <Renderer/GlCubemap.h>

void sf::GlMaterial::Create(const Material* material,
	const BufferLayout* vertexBufferLayout,
	const BufferLayout* voxelBufferLayout,
	const BufferLayout* particleBufferLayout)
{
	m_material = material;
	assert(m_material != nullptr);
	assert((m_material->vertShaderFilePath.length() > 0 || m_material->meshShaderFilePath.length() > 0) &&
		   (m_material->fragShaderFilePath.length() > 0));
	m_shader = new GlShader();
	m_shader->Create(*m_material, vertexBufferLayout, voxelBufferLayout, particleBufferLayout);

	for (const std::pair<std::string, Uniform>& uniformPair : m_material->uniforms)
	{
		if (uniformPair.second.dataType == DataType::bitmap)
		{
			GlTexture* newTexture = new GlTexture();
			newTexture->CreateFromBitmap(*((Bitmap*)uniformPair.second.data.p));
			m_textures[uniformPair.second.data.p] = newTexture;
		}
	}
}

void sf::GlMaterial::Bind(const std::vector<void*>& rendererUniformVector)
{
	if (m_material->isDoubleSided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	switch (m_material->drawMode)
	{
	case MaterialDrawMode::Fill:
		glPolygonMode(GL_FRONT, GL_FILL);
		break;
	case MaterialDrawMode::Lines:
		glPolygonMode(GL_FRONT, GL_LINE);
		break;
	case MaterialDrawMode::Points:
		glPolygonMode(GL_FRONT, GL_POINT);
		break;
	default:
		assert(!"Invalid draw mode");
		break;
	}

	switch (m_material->blendMode)
	{
	case MaterialBlendMode::Alpha:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		break;
	case MaterialBlendMode::Multiply:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ONE, GL_ONE);
		break;
	case MaterialBlendMode::Add:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
		break;
	default:
		assert(!"Invalid blend mode");
		break;
	}

	m_shader->Bind();

	for (const std::pair<std::string, Uniform>& uniform : m_material->uniforms)
	{
		int uniformTextureIndex;
		switch (uniform.second.dataType)
		{
		case DataType::bitmap:
			uniformTextureIndex = m_shader->GetOrAssignTextureIndex(uniform.first);
			if (uniform.second.data.p == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_2D, 0);
				m_shader->SetUniform1i(uniform.first, uniformTextureIndex);
			}
			else
			{
				GlTexture* texture = (GlTexture*)m_textures[uniform.second.data.p];
				texture->Bind(uniformTextureIndex);
				m_shader->SetUniform1i(uniform.first, uniformTextureIndex);
			}
			break;
		case DataType::cubemap:
			uniformTextureIndex = m_shader->GetOrAssignTextureIndex(uniform.first);
			if (uniform.second.data.p == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				m_shader->SetUniform1i(uniform.first, uniformTextureIndex);
			}
			else
			{
				GlCubemap* cubemap = (GlCubemap*)m_textures[uniform.second.data.p];
				cubemap->Bind(uniformTextureIndex);
				m_shader->SetUniform1i(uniform.first, uniformTextureIndex);
			}
			break;
		case DataType::b:
		case DataType::f32:
			m_shader->SetUniform1f(uniform.first, uniform.second.data.f32);
			break;
		case DataType::i32:
			m_shader->SetUniform1i(uniform.first, uniform.second.data.i32);
			break;
		case DataType::u32:
			m_shader->SetUniform1u(uniform.first, uniform.second.data.u32);
			break;
		case DataType::vec2f32:
			m_shader->SetUniform2fv(uniform.first, (float*)uniform.second.data.p);
			break;
		case DataType::vec3f32:
			m_shader->SetUniform3fv(uniform.first, (float*)uniform.second.data.p);
			break;
		case DataType::vec4f32:
			m_shader->SetUniform4fv(uniform.first, (float*)uniform.second.data.p);
			break;
		case DataType::vec2i32:
			m_shader->SetUniform2iv(uniform.first, (int32_t*)uniform.second.data.p);
			break;
		case DataType::vec3i32:
			m_shader->SetUniform3iv(uniform.first, (int32_t*)uniform.second.data.p);
			break;
		case DataType::vec4i32:
			m_shader->SetUniform4iv(uniform.first, (int32_t*)uniform.second.data.p);
			break;
		case DataType::vec2u32:
			m_shader->SetUniform2uiv(uniform.first, (uint32_t*)uniform.second.data.p);
			break;
		case DataType::vec3u32:
			m_shader->SetUniform3uiv(uniform.first, (uint32_t*)uniform.second.data.p);
			break;
		case DataType::vec4u32:
			m_shader->SetUniform4uiv(uniform.first, (uint32_t*)uniform.second.data.p);
			break;
		default:
			assert(!"Data type not handled");
			break;
		}
	}

	for (const std::pair<std::string, RendererUniform>& uniform : m_material->rendererUniforms)
	{
		int uniformTextureIndex;
		switch (uniform.second.dataType)
		{
		case DataType::bitmap:
			uniformTextureIndex = m_shader->GetOrAssignTextureIndex(uniform.first);
			if (rendererUniformVector[(uint32_t)uniform.second.data] == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_2D, 0);
				m_shader->SetUniform1i(uniform.first, uniformTextureIndex);
			}
			else
			{
				GlTexture* texture = (GlTexture*)rendererUniformVector[(uint32_t)uniform.second.data];
				texture->Bind(uniformTextureIndex);
				m_shader->SetUniform1i(uniform.first, uniformTextureIndex);
			}
			break;
		case DataType::cubemap:
			uniformTextureIndex = m_shader->GetOrAssignTextureIndex(uniform.first);
			if (rendererUniformVector[(uint32_t)uniform.second.data] == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				m_shader->SetUniform1i(uniform.first, uniformTextureIndex);
			}
			else
			{
				GlCubemap* cubemap = (GlCubemap*)rendererUniformVector[(uint32_t)uniform.second.data];
				cubemap->Bind(uniformTextureIndex);
				m_shader->SetUniform1i(uniform.first, uniformTextureIndex);
			}
			break;
		default:
			assert(!"Data type not handled");
			break;
		}
	}
}