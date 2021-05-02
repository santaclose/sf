#include "Material.h"

#include <iostream>

#include <Renderer/Texture.h>
#include <Renderer/Cubemap.h>

void sf::Material::CreateFromShader(Shader* theShader, bool isDoubleSided)
{
	m_shader = theShader;
	m_isDoubleSided = isDoubleSided;
}

void sf::Material::SetUniform(const std::string& name, void* data, UniformType type)
{
	m_uniformNames.push_back(name);
	m_uniformData.push_back(data);
	m_uniformTypes.push_back(type);

	if (type == UniformType::_Texture || type == UniformType::_Cubemap)
		m_shader->AssignTextureNumberToUniform(name);
}

void sf::Material::Bind()
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
			Texture* currentTexture = (Texture*)m_uniformData[i];
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
			Cubemap* currentCubemap = (Cubemap*)m_uniformData[i];
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
