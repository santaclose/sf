#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "HdrTexture.h"

class Cubemap
{
private:
	unsigned int m_gl_id;
	bool m_isHdr;

public:
	void CreateFromFiles(const std::vector<std::string>& files, bool isHdr = false);
	void CreateFromFiles(const std::string& name, const std::string& extension, bool isHdr = false);
	bool IsHDR();

	//void CreateFomHDR(const HdrTexture& hdrTexture);
	//void CreateIrradiance(const Cubemap& other);

	~Cubemap();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;
};