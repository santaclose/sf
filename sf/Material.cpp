#include "Material.h"
#include "Texture.h"

Material* Material::boundMaterial = nullptr;

Material::Material(Shader* theShader)
{
	m_shader = theShader;
}

void Material::SetUniform(const std::string& name, void* data, UniformType type)
{
	m_uniformNames.push_back(name);
	m_uniformData.push_back(data);
	m_uniformTypes.push_back(type);
}

void Material::Bind()
{
	if (boundMaterial == this) // no need to bind
		return;

	if (boundMaterial == nullptr || boundMaterial->m_shader != m_shader)
		m_shader->Bind();

	int textureCounter = 0;
	for (int i = 0; i < m_uniformData.size(); i++)
	{
		switch (m_uniformTypes[i])
		{
		case UniformType::_Texture:
			Texture* currentTexture = (Texture*)m_uniformData[i];
			currentTexture->Bind(textureCounter);
			m_shader->SetUniform1i(m_uniformNames[i], textureCounter);

			textureCounter++;
			break;
		}
	}
	boundMaterial = this;
}
