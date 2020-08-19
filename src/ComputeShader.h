#pragma once
#include <string>

class ComputeShader
{
private:
	unsigned int m_gl_id;
public:
	ComputeShader();
	~ComputeShader();
	void CreateFromFile(const std::string& shaderPath);
	void Bind() const;
};

