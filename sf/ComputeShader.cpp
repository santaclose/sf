#include "ComputeShader.h"
#include "Shader.h"
#include <fstream>
#include <iostream>

ComputeShader::ComputeShader() : m_gl_id(-1) {}

ComputeShader::~ComputeShader()
{
	glDeleteProgram(m_gl_id);
}

void ComputeShader::CreateFromFile(const std::string& shaderPath)
{
	std::ifstream ifs(shaderPath);
	
	if (ifs.fail())
		std::cout << "Could not read vertex shader file: " << shaderPath << std::endl;

	std::string source((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	m_gl_id = glCreateProgram();
	unsigned int s = Shader::CompileShader(GL_COMPUTE_SHADER, source);

	glAttachShader(m_gl_id, s);
	glLinkProgram(m_gl_id);
	glValidateProgram(m_gl_id);

	glDeleteShader(s);
}

void ComputeShader::Bind() const
{
	glUseProgram(m_gl_id);
}
