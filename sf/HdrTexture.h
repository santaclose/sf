#pragma once

#include <string>

class Cubemap;

class HdrTexture
{
	friend Cubemap;

	unsigned int m_gl_id;
	float* m_localBuffer;
	int m_width, m_height, m_BPP;

public:
	void CreateFromFile(const std::string& filePath);
	~HdrTexture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
};

