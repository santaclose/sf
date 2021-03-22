#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Cubemap
{
private:
	unsigned int m_gl_id;
	int m_size;
	bool m_isHdr;

public:
	void Create(unsigned int size, bool mipmap = true, bool isHdr = false);
	void CreateFromFiles(const std::vector<std::string>& files, bool mipmap = true);
	void CreateFromFiles(const std::string& name, const std::string& extension, bool mipmap = true);
	void ComputeMipmap();

	//void CreateFomHDR(const Texture& hdrTexture);

	~Cubemap();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline unsigned int GlId() const { return m_gl_id; }
	inline bool IsHDR() const { return m_isHdr; }
	inline int GetSize() const { return m_size; }
};