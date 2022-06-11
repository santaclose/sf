#include "GlShader.h"

#include <iostream>
#include <string>
#include <fstream>

unsigned int sf::GlShader::CompileShader(unsigned int type, const std::string& source)
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
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

sf::GlShader::GlShader() : m_gl_id(-1) {}

void sf::GlShader::CreateFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	m_vertFileName = vertexShaderPath;
	m_fragFileName = fragmentShaderPath;

	if (m_gl_id != -1)
		glDeleteProgram(m_gl_id);

	std::ifstream ifs(vertexShaderPath);
	std::ifstream ifs2(fragmentShaderPath);

	if (ifs.fail())
		std::cout << "[Shader] Could not read vertex shader file: " << vertexShaderPath << std::endl;
	if (ifs2.fail())
		std::cout << "[Shader] Could not read fragment shader file: " << fragmentShaderPath << std::endl;

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

sf::GlShader::~GlShader()
{
	glDeleteProgram(m_gl_id);
}

void sf::GlShader::Bind() const
{
	glUseProgram(m_gl_id);
	//std::cout << "Shader " << m_gl_id << " bound\n";
}

int sf::GlShader::GetUniformLocation(const std::string& name)
{
	if (m_uniformCache.find(name) != m_uniformCache.end() && m_uniformCache[name].location != -1)
		return m_uniformCache[name].location;

	int location = glGetUniformLocation(m_gl_id, name.c_str());
	//if (location == -1)
	//	std::cout << "[Shader] Could not get uniform location for " << name << " in shader " << m_vertFileName << "-" << m_fragFileName << std::endl;

	m_uniformCache[name].location = location;
	return location;
}

void sf::GlShader::AssignTextureNumberToUniform(const std::string& name)
{
	if (m_uniformCache[name].textureIndex == -1)
	{
		m_uniformCache[name].textureIndex = m_textureIndexCounter;
		m_textureIndexCounter++;
	}
}

int sf::GlShader::GetTextureIndex(const std::string& name)
{
	return m_uniformCache[name].textureIndex;
}

void sf::GlShader::SetUniformMatrix4fv(const std::string& name, const float* pointer, unsigned int number)
{
	glUniformMatrix4fv(GetUniformLocation(name), number, GL_FALSE, pointer);
}
void sf::GlShader::SetUniform1fv(const std::string& name, const float* pointer, unsigned int number)
{
	glUniform1fv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform3fv(const std::string& name, const float* pointer, unsigned int number)
{
	glUniform3fv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform4fv(const std::string& name, const float* pointer, unsigned int number)
{
	glUniform4fv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform1i(const std::string& name, int value)
{
	glUniform1i(GetUniformLocation(name), value);
}

void sf::GlShader::SetUniform1f(const std::string& name, float value)
{
	glUniform1f(GetUniformLocation(name), value);
}
