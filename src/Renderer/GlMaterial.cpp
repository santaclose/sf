#include "GlMaterial.h"

#include <iostream>

#include <Renderer/GlTexture.h>
#include <Renderer/GlCubemap.h>

void sf::GlMaterial::Create(const Material& material, const std::vector<void*>& rendererUniformVector, const BufferLayout& vertexBufferLayout, const BufferLayout* voxelBufferLayout, const BufferLayout* particleBufferLayout)
{
	assert(material.vertexShaderFilePath.length() > 0 && material.fragmentShaderFilePath.length() > 0);
	m_shader = new GlShader();
	m_shader->CreateFromFiles(material.vertexShaderFilePath, material.fragmentShaderFilePath, vertexBufferLayout, voxelBufferLayout, particleBufferLayout);
	m_isDoubleSided = material.isDoubleSided;
	m_drawMode = material.drawMode;
	m_blendMode = material.blendMode;

	for (const std::pair<std::string, Uniform>& uniformPair : material.uniforms)
	{
		switch (uniformPair.second.dataType)
		{
		case (uint32_t)DataType::b:
			SetUniform(uniformPair.first, uniformPair.second.data, UniformType::_1i);
			break;
		case (uint32_t)DataType::vec2f32:
			SetUniform(uniformPair.first, uniformPair.second.data, UniformType::_2f);
			break;
		case (uint32_t)DataType::vec3f32:
			SetUniform(uniformPair.first, uniformPair.second.data, UniformType::_3f);
			break;
		case (uint32_t)DataType::vec4f32:
			SetUniform(uniformPair.first, uniformPair.second.data, UniformType::_4f);
			break;
		case (uint32_t)ShaderDataType::bitmap:
			GlTexture* newTexture = new GlTexture();
			newTexture->CreateFromBitmap(*((Bitmap*)uniformPair.second.data));
			SetUniform(uniformPair.first, newTexture, UniformType::_Texture);
			break;
		}
	}

	for (const std::pair<std::string, RendererUniform>& uniformPair : material.rendererUniforms)
	{
		switch (uniformPair.second.dataType)
		{
		case (uint32_t)DataType::b:
			SetUniform(uniformPair.first, rendererUniformVector[(uint32_t)uniformPair.second.data], UniformType::_1i);
			break;
		case (uint32_t)ShaderDataType::bitmap:
			SetUniform(uniformPair.first, rendererUniformVector[(uint32_t)uniformPair.second.data], UniformType::_Texture);
			break;
		case (uint32_t)ShaderDataType::cubemap:
			SetUniform(uniformPair.first, rendererUniformVector[(uint32_t)uniformPair.second.data], UniformType::_Cubemap);
			break;
		}
	}
}

void sf::GlMaterial::CreateFromShader(GlShader* theShader, bool isDoubleSided, MaterialDrawMode drawMode, MaterialBlendMode blendMode)
{
	m_shader = theShader;
	m_isDoubleSided = isDoubleSided;
	m_drawMode = drawMode;
	m_blendMode = blendMode;
}

void sf::GlMaterial::SetUniform(const std::string& name, void* data, UniformType type)
{
	m_uniformNames.push_back(name);
	m_uniformData.push_back(data);
	m_uniformTypes.push_back(type);

	if (type == UniformType::_Texture || type == UniformType::_Cubemap)
		m_shader->AssignTextureNumberToUniform(name);
}

void sf::GlMaterial::Bind()
{
	if (m_isDoubleSided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	switch (m_drawMode)
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

	}

	switch (m_blendMode)
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
	}

	m_shader->Bind();

	for (int i = 0; i < m_uniformData.size(); i++)
	{
		int uniformTextureIndex = m_shader->GetTextureIndex(m_uniformNames[i]);
		switch (m_uniformTypes[i])
		{
		case UniformType::_Texture:
		{
			if (m_uniformData[i] == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_2D, 0);
				m_shader->SetUniform1i(m_uniformNames[i], uniformTextureIndex);
				continue;
			}
			GlTexture* currentTexture = (GlTexture*)m_uniformData[i];
			currentTexture->Bind(uniformTextureIndex);
			m_shader->SetUniform1i(m_uniformNames[i], uniformTextureIndex);
			break;
		}
		case UniformType::_Cubemap:
		{
			if (m_uniformData[i] == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				m_shader->SetUniform1i(m_uniformNames[i], uniformTextureIndex);
				continue;
			}
			GlCubemap* currentCubemap = (GlCubemap*)m_uniformData[i];
			currentCubemap->Bind(uniformTextureIndex);
			m_shader->SetUniform1i(m_uniformNames[i], uniformTextureIndex);
			break;
		}
		case UniformType::_1i:
		{
			m_shader->SetUniform1i(m_uniformNames[i], (int)(unsigned long long)m_uniformData[i]);
			break;
		}
		case UniformType::_2f:
		{
			m_shader->SetUniform2fv(m_uniformNames[i], (float*)m_uniformData[i]);
			break;
		}
		case UniformType::_3f:
		{
			m_shader->SetUniform3fv(m_uniformNames[i], (float*)m_uniformData[i]);
			break;
		}
		case UniformType::_4f:
		{
			m_shader->SetUniform4fv(m_uniformNames[i], (float*)m_uniformData[i]);
			break;
		}
		}
	}
}
