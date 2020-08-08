#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "HdrTexture.h"

class Cubemap
{
private:
	unsigned int m_gl_id;

	static glm::mat4 captureProjection;
	static glm::mat4 captureViews[];

public:
	void CreateFromFiles(const std::vector<std::string>& files);
	void CreateFromFiles(const std::string& name, const std::string& extension);

	void CreateFomHDR(const HdrTexture& hdrTexture);

	void CreateIrradiance(const Cubemap& other);
	~Cubemap();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;
};