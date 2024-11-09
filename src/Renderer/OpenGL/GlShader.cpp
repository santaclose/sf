#include "GlShader.h"

#include <FileUtils.h>

#include <glad/glad.h>
#include <iostream>
#include <string>

uint32_t sf::GlShader::CompileShader(uint32_t type, const std::string& source)
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
	uint32_t id = glCreateShader(type);
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
		std::cout << "[GlShader] Failed to compile " << messageType << " shader" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

sf::GlShader::GlShader() : gl_id(-1) {}

void sf::GlShader::CreateFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	std::cout << "[GlShader] Creating shader from files: " << vertexShaderPath << ", " << fragmentShaderPath << std::endl;
	m_vertFileName = vertexShaderPath + ".glsl";
	m_fragFileName = fragmentShaderPath + ".glsl";

	Delete();

	std::string vertexShaderSource, fragmentShaderSource;
	if (!FileUtils::ReadFileAsString(m_vertFileName, vertexShaderSource))
		std::cout << "[GlShader] Could not read vertex shader file: " << vertexShaderPath << std::endl;
	if (!FileUtils::ReadFileAsString(m_fragFileName, fragmentShaderSource))
		std::cout << "[GlShader] Could not read fragment shader file: " << fragmentShaderPath << std::endl;

	gl_id = glCreateProgram();
	std::cout << "[GlShader] Created program with id " << gl_id << std::endl;
	uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	glAttachShader(gl_id, vs);
	glAttachShader(gl_id, fs);
	glLinkProgram(gl_id);
	glValidateProgram(gl_id);

	glDeleteShader(vs);
	glDeleteShader(fs);
}

void sf::GlShader::CreateComputeFromFile(const std::string& computeShaderPath)
{
	std::cout << "[GlShader] Creating compute shader from file: " << computeShaderPath << std::endl;
	std::string computeShaderSource;
	if (!FileUtils::ReadFileAsString(computeShaderPath, computeShaderSource))
		std::cout << "[GlShader] Could not read compute shader file: " << computeShaderPath << std::endl;

	gl_id = glCreateProgram();
	std::cout << "[GlShader] Created program with id " << gl_id << std::endl;
	uint32_t cs = CompileShader(GL_COMPUTE_SHADER, computeShaderSource);
	glAttachShader(gl_id, cs);
	glLinkProgram(gl_id);
	glValidateProgram(gl_id);
	glDeleteShader(cs);
}

sf::GlShader::~GlShader()
{
	Delete();
}

void sf::GlShader::Bind() const
{
	glUseProgram(gl_id);
	//std::cout << "Shader " << gl_id << " bound\n";
}

int sf::GlShader::GetUniformLocation(const std::string& name)
{
	if (m_uniformCache.find(name) != m_uniformCache.end() && m_uniformCache[name].location != -1)
		return m_uniformCache[name].location;

	int location = glGetUniformLocation(gl_id, name.c_str());
	if (location == -1)
		std::cout << "[GlShader] Could not get uniform location for " << name << " in shader " << m_vertFileName << "-" << m_fragFileName << std::endl;

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

void sf::GlShader::Delete()
{
	if (gl_id != -1)
	{
		glDeleteProgram(gl_id);
		uint32_t boundProgram; glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&boundProgram);
		if (gl_id == boundProgram)
			glUseProgram(0); // can't leave deleted program bound
		std::cout << "[GlShader] Deleted program with id " << gl_id << std::endl;
		gl_id = -1;
	}
}

void sf::GlShader::SetUniformMatrix4fv(const std::string& name, const float* pointer, uint32_t number)
{
	glUniformMatrix4fv(GetUniformLocation(name), number, GL_FALSE, pointer);
}
void sf::GlShader::SetUniform1fv(const std::string& name, const float* pointer, uint32_t number)
{
	glUniform1fv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform2fv(const std::string& name, const float* pointer, uint32_t number)
{
	glUniform2fv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform3fv(const std::string& name, const float* pointer, uint32_t number)
{
	glUniform3fv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform4fv(const std::string& name, const float* pointer, uint32_t number)
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