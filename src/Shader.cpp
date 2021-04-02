#include "Shader.h"

#include <iostream>
#include <string>
#include <fstream>

unsigned int sf::Shader::CompileShader(unsigned int type, const std::string& source)
{
	std::string messageType;
	switch (type)
	{
	case GL_VERTEX_SHADER:
		messageType = "vertex";
		break;
	case GL_FRAGMENT_SHADER:
		messageType = "fragment";
		break;
	case GL_COMPUTE_SHADER:
		messageType = "compute";
		break;
	default:
		messageType = "unknown";
		break;
	}
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "[Shader] Failed to compile " << messageType << " shader" << std::endl;
		//std::cout << "----------------------------\n";
		//std::cout << source << std::endl;
		//std::cout << "----------------------------\n";
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

sf::Shader::Shader() : m_gl_id(-1) {}

void sf::Shader::CreateFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	if (m_gl_id != -1)
		glDeleteProgram(m_gl_id);

	std::ifstream ifs(vertexShaderPath);
	std::ifstream ifs2(fragmentShaderPath);

	if (ifs.fail())
		std::cout << "Could not read vertex shader file: " << vertexShaderPath << std::endl;
	if (ifs2.fail())
		std::cout << "Could not read fragment shader file: " << fragmentShaderPath << std::endl;

	std::string vertexShaderSource((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	std::string fragmentShaderSource((std::istreambuf_iterator<char>(ifs2)),
		(std::istreambuf_iterator<char>()));

	m_gl_id = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	glAttachShader(m_gl_id, vs);
	glAttachShader(m_gl_id, fs);
	glLinkProgram(m_gl_id);
	glValidateProgram(m_gl_id);

	glDeleteShader(vs);
	glDeleteShader(fs);
}

sf::Shader::~Shader()
{
	glDeleteProgram(m_gl_id);
}

void sf::Shader::Bind() const
{
	glUseProgram(m_gl_id);
	//std::cout << "Shader " << m_gl_id << " bound\n";
}

int sf::Shader::GetUniformLocation(const std::string& name)
{
	if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
		return m_uniformLocationCache[name];

	int location = glGetUniformLocation(m_gl_id, name.c_str());
	if (location == -1)
		std::cout << "Could not get uniform location for " << name << std::endl;

	m_uniformLocationCache[name] = location;
	return location;
}

void sf::Shader::SetUniformMatrix4fv(const std::string& name, const float* pointer, unsigned int number)
{
	glUniformMatrix4fv(GetUniformLocation(name), number, GL_FALSE, pointer);
}
void sf::Shader::SetUniform1fv(const std::string& name, const float* pointer, unsigned int number)
{
	glUniform1fv(GetUniformLocation(name), number, pointer);
}
void sf::Shader::SetUniform3fv(const std::string& name, const float* pointer, unsigned int number)
{
	glUniform3fv(GetUniformLocation(name), number, pointer);
}
void sf::Shader::SetUniform4fv(const std::string& name, const float* pointer, unsigned int number)
{
	glUniform4fv(GetUniformLocation(name), number, pointer);
}
void sf::Shader::SetUniform1i(const std::string& name, int value)
{
	glUniform1i(GetUniformLocation(name), value);
}

void sf::Shader::SetUniform1f(const std::string& name, float value)
{
	glUniform1f(GetUniformLocation(name), value);
}
