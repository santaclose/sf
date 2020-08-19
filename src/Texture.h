#pragma once

#include <string>

class Texture
{
private:
	unsigned int m_gl_id;
	unsigned char* m_localBuffer;
	int m_width, m_height, m_BPP;
public:
	enum Type {
		Albedo = 0, Normals = 1, Roughness = 2, Metallic = 3
		//ColorData, NonColorData
	};

	//Texture(const std::string& path, Type t);
	void CreateFromFile(const std::string& path, Type t);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
};