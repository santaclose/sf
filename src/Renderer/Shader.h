#pragma once

#include <string>
#include <glad/glad.h>
#include <unordered_map>

namespace sf {

	class ComputeShader;
	class Material;

	struct ShaderUniformData {
		int location = -1;
		int textureIndex = -1;
	};

	class Shader
	{
		friend ComputeShader;
		friend Material;
	private:
		std::string m_vertFileName;
		std::string m_fragFileName;
		std::unordered_map<std::string, ShaderUniformData> m_uniformCache;
		int m_textureIndexCounter = 0;
	public:
		unsigned int m_gl_id;
	private:
		static unsigned int CompileShader(unsigned int type, const std::string& source);
		int GetUniformLocation(const std::string& name);
		void AssignTextureNumberToUniform(const std::string& name);
		int GetTextureIndex(const std::string& name);
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
}