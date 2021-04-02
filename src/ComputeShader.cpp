#include "ComputeShader.h"

#include <fstream>
#include <iostream>

#include <Shader.h>

sf::ComputeShader::ComputeShader() : m_gl_id(-1) {}

sf::ComputeShader::~ComputeShader()
{
	glDeleteProgram(m_gl_id);
}

void sf::ComputeShader::CreateFromFile(const std::string& shaderPath)
{
	std::ifstream ifs(shaderPath);
	
	if (ifs.fail())
		std::cout << "Could not read vertex shader file: " << shaderPath << std::endl;

	std::string source((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	const GLchar* srcBufferPtr = source.c_str();

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &srcBufferPtr, nullptr);
	glCompileShader(shader);

	{
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE) {
			GLsizei infoLogSize;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
			std::unique_ptr<GLchar[]> infoLog(new GLchar[infoLogSize]);
			glGetShaderInfoLog(shader, infoLogSize, nullptr, infoLog.get());
			std::cout << "[Compute shader] Failed to compile compute shader: " + shaderPath + "\n" + infoLog.get() << std::endl;
		}
	}

	m_gl_id = glCreateProgram();
	glAttachShader(m_gl_id, shader);
	glLinkProgram(m_gl_id);
	glDetachShader(m_gl_id, shader);
	glDeleteShader(shader);

	{
		GLint status;
		glGetProgramiv(m_gl_id, GL_LINK_STATUS, &status);
		if (status == GL_TRUE) {
			glValidateProgram(m_gl_id);
			glGetProgramiv(m_gl_id, GL_VALIDATE_STATUS, &status);
		}
		if (status != GL_TRUE) {
			GLsizei infoLogSize;
			glGetProgramiv(m_gl_id, GL_INFO_LOG_LENGTH, &infoLogSize);
			std::unique_ptr<GLchar[]> infoLog(new GLchar[infoLogSize]);
			glGetProgramInfoLog(m_gl_id, infoLogSize, nullptr, infoLog.get());
			std::cout << "[Compute shader] Failed to link compute shader: " + shaderPath << "\n" << infoLog.get() << std::endl;
		}
	}
}

void sf::ComputeShader::Bind() const
{
	glUseProgram(m_gl_id);
}
