#pragma once

#include <string>
#include <glad/glad.h>
#include <unordered_map>

namespace sf {

	class ComputeShader;
	class GlMaterial;

	struct ShaderUniformData {
		int location = -1;
		int textureIndex = -1;
	};

	class GlShader
	{
		friend ComputeShader;
		friend GlMaterial;
	private:
		std::string m_vertFileName;
		std::string m_fragFileName;
		std::unordered_map<std::string, ShaderUniformData> m_uniformCache;
		int m_textureIndexCounter = 0;
	public:
		uint32_t gl_id;
	private:
		static uint32_t CheckLinkStatusAndReturnProgram(uint32_t program, bool outputErrorMessages);
		static uint32_t CompileShader(uint32_t type, const std::string& source);
		int GetUniformLocation(const std::string& name);
		void AssignTextureNumberToUniform(const std::string& name);
		int GetTextureIndex(const std::string& name);
		void Delete();
	public:
		GlShader();
		~GlShader();
		void CreateFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		void CreateComputeFromFile(const std::string& computeShaderPath);
		inline bool Initialized() { return gl_id != -1; };
		void Bind() const;
		void SetUniformMatrix4fv(const std::string& name, const float* pointer, uint32_t number = 1);
		void SetUniform1fv(const std::string& name, const float* pointer, uint32_t number = 1);
		void SetUniform2fv(const std::string& name, const float* pointer, uint32_t number = 1);
		void SetUniform3fv(const std::string& name, const float* pointer, uint32_t number = 1);
		void SetUniform4fv(const std::string& name, const float* pointer, uint32_t number = 1);
		void SetUniform1i(const std::string& name, int value);
		void SetUniform1f(const std::string& name, float value);
	};
}