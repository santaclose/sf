#pragma once

#include <string>

class Texture
{
private:
	unsigned int m_id;
	unsigned char* m_localBuffer;
	int m_width, m_height, m_BPP;
public:
	enum Type {
		Albedo, NormalMap
	};

	Texture(const std::string& path, Type t);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
};