#pragma once

#include <string>
#include <glad/glad.h>
#include <unordered_map>
class ComputeShader;

class Shader
{
	friend ComputeShader;
private:
	std::unordered_map<std::string, int> m_uniformLocationCache;
public:
	unsigned int m_gl_id;
private:
	static unsigned int CompileShader(unsigned int type, const std::string& source);
	int GetUniformLocation(const std::string& name);
public:
	//Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	Shader();
	~Shader();
	void CreateFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	void Bind() const;
	void SetUniformMatrix4fv(const std::string& name, const float* pointer, unsigned int number = 1);
	void SetUniform1fv(const std::string& name, const float* pointer, unsigned int number = 1);
	void SetUniform3fv(const std::string& name, const float* pointer, unsigned int number = 1);
	void SetUniform4fv(const std::string& name, const float* pointer, unsigned int number = 1);
	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
};