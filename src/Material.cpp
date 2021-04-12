#include "Material.h"

#include <Texture.h>
#include <Cubemap.h>

sf::Material* sf::Material::boundMaterial = nullptr;

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
}

void sf::Material::Bind()
{
	if (m_isDoubleSided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	m_shader->Bind();

	int textureCounter = 0;

	for (int i = 0; i < m_uniformData.size(); i++)
	{
		switch (m_uniformTypes[i])
		{
		case UniformType::_Texture:
		{
			if (m_uniformData[i] == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + textureCounter);
				glBindTexture(GL_TEXTURE_2D, 0);
				m_shader->SetUniform1i(m_uniformNames[i], textureCounter);
				continue;
			}
			Texture* currentTexture = (Texture*)m_uniformData[i];
			currentTexture->Bind(textureCounter);
			m_shader->SetUniform1i(m_uniformNames[i], textureCounter);

			textureCounter++;
			break;
		}
		case UniformType::_Cubemap:
		{
			if (m_uniformData[i] == nullptr) // clear uniform if not provided
			{
				glActiveTexture(GL_TEXTURE0 + textureCounter);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				m_shader->SetUniform1i(m_uniformNames[i], textureCounter);
				continue;
			}
			Cubemap* currentCubemap = (Cubemap*)m_uniformData[i];
			currentCubemap->Bind(textureCounter);
			m_shader->SetUniform1i(m_uniformNames[i], textureCounter);

			textureCounter++;
			break;
		}
		case UniformType::_Color:
		{
			m_shader->SetUniform4fv(m_uniformNames[i], (float*)m_uniformData[i]);
			break;
		}
		case UniformType::_Boolean:
		{
			m_shader->SetUniform1i(m_uniformNames[i], (int)m_uniformData[i]);
			break;
		}
		}
	}
	boundMaterial = this;
}
