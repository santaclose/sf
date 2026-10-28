#include "GlMaterial.h"

#include <iostream>

#include <Renderer/GlTexture.h>
#include <Renderer/GlCubemap.h>

void sf::GlMaterial::Create(const Material* material, Buffer::Layout vertexBufferLayout)
{
	m_material = material;
	assert(m_material != nullptr);
	assert((m_material->vertShaderFilePath.length() > 0 || m_material->meshShaderFilePath.length() > 0) &&
		   (m_material->fragShaderFilePath.length() > 0));
	m_shader = new GlShader();
	m_shader->Create(*m_material, vertexBufferLayout);

	for (const std::pair<std::string, Uniform>& uniformPair : m_material->uniforms)
	{
		const std::string& name = uniformPair.first;
		const Uniform& unif = uniformPair.second;
		if (uniformPair.second.dataType == DataType::bitmap && uniformPair.second.data.p != nullptr)
		{
			GlTexture* newTexture = new GlTexture();
			newTexture->CreateFromBitmap(*((Bitmap*)uniformPair.second.data.p));
			m_textures[uniformPair.second.data.p] = newTexture;
		}
	}

	for (int i = 0; i < m_material->buffers.size(); i++)
	{
		if (m_material->buffers[i].pointer == nullptr)
			continue;
		uint32_t newSsbo;
		glGenBuffers(1, &newSsbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, newSsbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_material->buffers[i].size, m_material->buffers[i].pointer, GL_STATIC_DRAW);
		m_ssbos[m_material->buffers[i].pointer] = newSsbo;
	}
}

void sf::GlMaterial::Delete()
{
	if (m_shader != nullptr)
	{
		m_shader->Delete();
		delete m_shader;
	}
	for (const std::pair<const void*, GlTexture*>& tx : m_textures)
	{
		tx.second->Delete();
		delete tx.second;
	}
	for (const std::pair<const void*, uint32_t>& ssbo : m_ssbos)
	{
		glDeleteBuffers(1, &ssbo.second);
	}
}

void sf::GlMaterial::Bind(const std::vector<void*>& rendererUniformVector, const std::unordered_map<std::string, UniformData>* uniformOverrides)
{
	if (m_material->usesAlphaToCoverage)
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	else
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

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
	case MaterialBlendMode::None:
		glDisable(GL_BLEND);
		break;
	case MaterialBlendMode::Alpha:
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		break;
	case MaterialBlendMode::Multiply:
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ONE, GL_ONE);
		break;
	case MaterialBlendMode::Add:
		glEnable(GL_BLEND);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
		break;
	default:
		assert(!"Invalid blend mode");
		break;
	}

	m_shader->Bind();

	for (const std::pair<std::string, Uniform>& uniformPair : m_material->uniforms)
	{
		int uniformTextureIndex;
		GlTexture* texture = nullptr;
		const std::string& name = uniformPair.first;
		Uniform unif = uniformPair.second;
		if (uniformOverrides != nullptr && uniformOverrides->find(name) != uniformOverrides->end())
			unif.data = uniformOverrides->at(name);
		switch (unif.dataType)
		{
		case DataType::bitmap:
			uniformTextureIndex = m_shader->GetOrAssignTextureIndex(name);
			if (m_textures.find(unif.data.p) != m_textures.end())
				texture = (GlTexture*) m_textures[unif.data.p];
			else if (unif.data.p != nullptr)
			{
				GlTexture* newTexture = new GlTexture();
				newTexture->CreateFromBitmap(*((Bitmap*)unif.data.p));
				m_textures[unif.data.p] = newTexture;
				texture = newTexture;
			}
			if (unif.data.p == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				if (texture != nullptr)
					glBindTexture(texture->GetTarget(), 0);
				m_shader->SetUniform1i(name, uniformTextureIndex);
			}
			else
			{
				texture->Bind(uniformTextureIndex);
				m_shader->SetUniform1i(name, uniformTextureIndex);
			}
			break;
		case DataType::cubemap:
			uniformTextureIndex = m_shader->GetOrAssignTextureIndex(name);
			if (unif.data.p == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				m_shader->SetUniform1i(name, uniformTextureIndex);
			}
			else
			{
				GlCubemap* cubemap = (GlCubemap*)m_textures[unif.data.p];
				cubemap->Bind(uniformTextureIndex);
				m_shader->SetUniform1i(name, uniformTextureIndex);
			}
			break;
		case DataType::b:
		case DataType::f32:
			m_shader->SetUniform1f(name, unif.data.f32);
			break;
		case DataType::i32:
			m_shader->SetUniform1i(name, unif.data.i32);
			break;
		case DataType::u32:
			m_shader->SetUniform1u(name, unif.data.u32);
			break;
		case DataType::vec2f32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform2fv(name, (float*)unif.data.p);
			break;
		case DataType::vec3f32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform3fv(name, (float*)unif.data.p);
			break;
		case DataType::vec4f32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform4fv(name, (float*)unif.data.p);
			break;
		case DataType::vec2i32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform2iv(name, (int32_t*)unif.data.p);
			break;
		case DataType::vec3i32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform3iv(name, (int32_t*)unif.data.p);
			break;
		case DataType::vec4i32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform4iv(name, (int32_t*)unif.data.p);
			break;
		case DataType::vec2u32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform2uiv(name, (uint32_t*)unif.data.p);
			break;
		case DataType::vec3u32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform3uiv(name, (uint32_t*)unif.data.p);
			break;
		case DataType::vec4u32:
			if (unif.data.p != nullptr)
				m_shader->SetUniform4uiv(name, (uint32_t*)unif.data.p);
			break;
		default:
			assert(!"Data type not handled");
			break;
		}
	}

	for (const std::pair<std::string, RendererUniform>& uniformPair : m_material->rendererUniforms)
	{
		int uniformTextureIndex;
		GlTexture* texture;
		switch (uniformPair.second.dataType)
		{
		case DataType::bitmap:
			uniformTextureIndex = m_shader->GetOrAssignTextureIndex(uniformPair.first);
			texture = (GlTexture*)rendererUniformVector[(uint32_t)uniformPair.second.data];
			if (rendererUniformVector[(uint32_t)uniformPair.second.data] == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(texture->GetTarget(), 0);
				m_shader->SetUniform1i(uniformPair.first, uniformTextureIndex);
			}
			else
			{
				texture->Bind(uniformTextureIndex);
				m_shader->SetUniform1i(uniformPair.first, uniformTextureIndex);
			}
			break;
		case DataType::cubemap:
			uniformTextureIndex = m_shader->GetOrAssignTextureIndex(uniformPair.first);
			if (rendererUniformVector[(uint32_t)uniformPair.second.data] == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				m_shader->SetUniform1i(uniformPair.first, uniformTextureIndex);
			}
			else
			{
				GlCubemap* cubemap = (GlCubemap*)rendererUniformVector[(uint32_t)uniformPair.second.data];
				cubemap->Bind(uniformTextureIndex);
				m_shader->SetUniform1i(uniformPair.first, uniformTextureIndex);
			}
			break;
		default:
			assert(!"Data type not handled");
			break;
		}
	}

	for (int i = 0; i < m_material->buffers.size(); i++)
	{
		if (m_material->buffers[i].pointer == nullptr)
			continue;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, m_ssbos[m_material->buffers[i].pointer]);
	}
}

void sf::GlMaterial::UpdateBufferData(uint32_t bufferIndex, uint32_t location, uint32_t size)
{
	assert(bufferIndex < m_material->buffers.size());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbos[m_material->buffers[bufferIndex].pointer]);
	if (location != ~0)
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, location, size, (uint8_t*) m_material->buffers[bufferIndex].pointer + location);
	else
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_material->buffers[bufferIndex].size, m_material->buffers[bufferIndex].pointer, GL_STATIC_DRAW);
}