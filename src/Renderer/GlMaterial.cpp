#include "GlMaterial.h"

#include <iostream>

#include <Renderer/GlTexture.h>
#include <Renderer/GlCubemap.h>

void sf::GlMaterial::Create(const Material& material, const std::vector<void*>& rendererUniformVector)
{
	m_shader = new GlShader();
	m_shader->CreateFromFiles(material.vertexShaderFilePath, material.fragmentShaderFilePath);
	m_isDoubleSided = material.isDoubleSided;

	for (const std::pair<std::string, Uniform>& uniformPair : material.uniforms)
	{
		switch (uniformPair.second.dataType)
		{
		case (uint32_t)DataType::b:
			SetUniform(uniformPair.first, uniformPair.second.data, UniformType::_Boolean);
			break;
		case (uint32_t)DataType::vec4f32:
			SetUniform(uniformPair.first, uniformPair.second.data, UniformType::_Color);
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
			SetUniform(uniformPair.first, rendererUniformVector[(uint32_t)uniformPair.second.data], UniformType::_Boolean);
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

void sf::GlMaterial::CreateFromShader(GlShader* theShader, bool isDoubleSided)
{
	m_shader = theShader;
	m_isDoubleSided = isDoubleSided;
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
				//std::cout << "clearing uniform: " << m_uniformNames[i] << std::endl;
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_2D, 0);
				m_shader->SetUniform1i(m_uniformNames[i], uniformTextureIndex);
				continue;
			}
			GlTexture* currentTexture = (GlTexture*)m_uniformData[i];
			currentTexture->Bind(uniformTextureIndex);
			m_shader->SetUniform1i(m_uniformNames[i], uniformTextureIndex);

			//std::cout << "setting value \"" << uniformTextureIndex << "\" to uniform \"" << m_uniformNames[i] << "\"\n";
			break;
		}
		case UniformType::_Cubemap:
		{
			if (m_uniformData[i] == nullptr) // clear uniform if not provided
			{
				//std::cout << "clearing uniform: " << m_uniformNames[i] << std::endl;
				glActiveTexture(GL_TEXTURE0 + uniformTextureIndex);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				m_shader->SetUniform1i(m_uniformNames[i], uniformTextureIndex);
				continue;
			}
			GlCubemap* currentCubemap = (GlCubemap*)m_uniformData[i];
			currentCubemap->Bind(uniformTextureIndex);
			m_shader->SetUniform1i(m_uniformNames[i], uniformTextureIndex);

			//std::cout << "setting value \"" << uniformTextureIndex << "\" to uniform \"" << m_uniformNames[i] << "\"\n";
			break;
		}
		case UniformType::_Color:
		{
			m_shader->SetUniform4fv(m_uniformNames[i], (float*)m_uniformData[i]);
			//std::cout << "setting color to uniform \"" << m_uniformNames[i] << "\"\n";
			break;
		}
		case UniformType::_Boolean:
		{
			m_shader->SetUniform1i(m_uniformNames[i], (int)m_uniformData[i]);
			//std::cout << "setting boolean \"" << (int)m_uniformData[i] << "\" to uniform \"" << m_uniformNames[i] << "\"\n";
			break;
		}
		}
	}
}
