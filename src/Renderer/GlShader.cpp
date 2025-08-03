#include "GlShader.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <cstring>

namespace
{
	void ResolveIncludes(std::string& shaderSource)
	{
		for (int i = 0; i < shaderSource.length(); i++)
		{
			if (strncmp(shaderSource.data() + i, "\n#include <", 11) == 0)
			{
				int j = i + 11;
				for (; shaderSource[j] != '>'; j++);
				std::string filePath = shaderSource.substr(i + 11, j - (i + 11));
				std::ifstream ifs(filePath);
				if (ifs.fail())
					std::cout << "[GlShader] Could not read shader include file: " << filePath << std::endl;
				std::string includeText((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
				shaderSource = shaderSource.substr(0, i + 1) + includeText + shaderSource.substr(j + 1);
			}
		}
	}

	std::string GenerateVertexAttributeShaderHeader(const sf::BufferLayout& vertexBufferLayout)
	{
		uint32_t currentLocation = 0;
		std::string out = "";
		for (const sf::BufferComponentInfo& bci : vertexBufferLayout.GetComponentInfos())
		{
			assert((uint32_t)bci.component < (uint32_t) sf::BufferComponent::VoxelPosition); // should be vertex component
			out += "layout(location = " + std::to_string(currentLocation) + ") in ";
			switch (bci.dataType)
			{
				case sf::DataType::f32:
					out += "float"; break;
				case sf::DataType::vec2f32:
					out += "vec2"; break;
				case sf::DataType::vec3f32:
					out += "vec3"; break;
				case sf::DataType::vec4f32:
					out += "vec4"; break;
				default:
					assert(false); // missing type, should add to this switch
			}
			out += " ";
			switch (bci.component)
			{
				case sf::BufferComponent::VertexPosition:
					out += "VA_Position;\n#define HAS_VA_Position 1\n"; break;
				case sf::BufferComponent::VertexNormal:
					out += "VA_Normal;\n#define HAS_VA_Normal 1\n"; break;
				case sf::BufferComponent::VertexTangent:
					out += "VA_Tangent;\n#define HAS_VA_Tangent 1\n"; break;
				case sf::BufferComponent::VertexColor:
					out += "VA_Color;\n#define HAS_VA_Color 1\n"; break;
				case sf::BufferComponent::VertexUV:
					out += "VA_UV;\n#define HAS_VA_UV 1\n"; break;
				case sf::BufferComponent::VertexAO:
					out += "VA_AO;\n#define HAS_VA_AO 1\n"; break;
				case sf::BufferComponent::VertexBoneWeights:
					out += "VA_BoneWeights;\n#define HAS_VA_BoneWeights 1\n"; break;
				case sf::BufferComponent::VertexBoneIndices:
					out += "VA_BoneIndices;\n#define HAS_VA_BoneIndices 1\n"; break;
			}
			currentLocation++;
		}
		return out;
	}

	std::string GenerateVoxelVolumeShaderHeader(const sf::BufferLayout& voxelBufferLayout)
	{
		std::string out = "";
		for (const sf::BufferComponentInfo& bci : voxelBufferLayout.GetComponentInfos())
		{
			assert(
				(uint32_t)bci.component < (uint32_t) sf::BufferComponent::ParticlePosition &&
				(uint32_t)bci.component >= (uint32_t) sf::BufferComponent::VoxelPosition); // should be voxel component
			// assume buffer is only floats
			uint32_t stride = voxelBufferLayout.GetSize() / 4u;
			uint32_t baseOffset = bci.byteOffset / 4u;
			switch (bci.component)
			{
			case sf::BufferComponent::VoxelPosition:
				out += "#define LOAD_VOXEL_POSITION vec3("
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 0) + "], "
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 1) + "], "
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 2) + "])\n";
				break;
			case sf::BufferComponent::VoxelNormal:
				out += "#define LOAD_VOXEL_NORMAL vec3("
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 0) + "], "
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 1) + "], "
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 2) + "])\n";
				break;
			case sf::BufferComponent::VoxelColor:
				out += "#define LOAD_VOXEL_COLOR vec3("
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 0) + "], "
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 1) + "], "
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 2) + "])\n";
				break;
			case sf::BufferComponent::VoxelUV:
				out += "#define LOAD_VOXEL_UV vec3("
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 0) + "], "
					"VOXEL_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 1) + "])\n";
				break;
			}
		}
		return out;
	}
}

uint32_t sf::GlShader::CheckLinkStatusAndReturnProgram(uint32_t program, bool outputErrorMessages)
{
	if (glGetError() != GL_NO_ERROR)
		return 0;

	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != 0)
		return program;
	if (outputErrorMessages)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		// Info log length includes the null terminator, so 1 means that the info log is an
		// empty string.
		if (infoLogLength > 1)
		{
			std::vector<GLchar> infoLog(infoLogLength);
			glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), nullptr,
				&infoLog[0]);
			std::cout << "[GlShader] Program link failed: " << &infoLog[0];
		}
		else
			std::cout << "[GlShader] Program link failed. <Empty log message>";
	}

	glDeleteProgram(program);
	return 0;
}

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

void sf::GlShader::CreateFromFiles(const std::string& vertexShaderPath, const std::string& fragmentShaderPath,
	const BufferLayout& vertexBufferLayout,
	const BufferLayout* voxelBufferLayout)
{
	m_vertFileName = vertexShaderPath + ".glsl";
	m_fragFileName = fragmentShaderPath + ".glsl";
	std::cout << "[GlShader] Creating shader from files: " << m_vertFileName << ", " << m_fragFileName << std::endl;

	Delete();

	std::ifstream ifs(m_vertFileName);
	std::ifstream ifs2(m_fragFileName);

	if (ifs.fail())
		std::cout << "[GlShader] Could not read vertex shader file: " << m_vertFileName << std::endl;
	if (ifs2.fail())
		std::cout << "[GlShader] Could not read fragment shader file: " << m_fragFileName << std::endl;

	std::string vertexShaderSource((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	std::string fragmentShaderSource((std::istreambuf_iterator<char>(ifs2)),
		(std::istreambuf_iterator<char>()));

	if (voxelBufferLayout != nullptr)
		vertexShaderSource = GenerateVoxelVolumeShaderHeader(*voxelBufferLayout) + vertexShaderSource;
	vertexShaderSource = GenerateVertexAttributeShaderHeader(vertexBufferLayout) + vertexShaderSource;
	vertexShaderSource = "#version 460\n" + vertexShaderSource;

	fragmentShaderSource = "#version 460\nlayout(location = 0) out vec4 OUT_COLOR;\n" + fragmentShaderSource;

	ResolveIncludes(vertexShaderSource);
	ResolveIncludes(fragmentShaderSource);

	gl_id = glCreateProgram();
	std::cout << "[GlShader] Created program with id " << gl_id << std::endl;
	uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	if (!vs)
		std::cout << vertexShaderSource << std::endl;
	if (!fs)
		std::cout << fragmentShaderSource << std::endl;

	glAttachShader(gl_id, vs);
	glAttachShader(gl_id, fs);
	glLinkProgram(gl_id);
	CheckLinkStatusAndReturnProgram(gl_id, true);
	glValidateProgram(gl_id);

	glDeleteShader(vs);
	glDeleteShader(fs);
}

void sf::GlShader::CreateComputeFromFile(const std::string& computeShaderPath)
{
	std::string computeShaderPathGlsl = computeShaderPath + ".glsl";
	std::cout << "[GlShader] Creating compute shader from file: " << computeShaderPathGlsl << std::endl;
	std::ifstream ifs(computeShaderPathGlsl);
	if (ifs.fail())
		std::cout << "[GlShader] Could not read compute shader file: " << computeShaderPathGlsl << std::endl;
	std::string computeShaderSource((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	ResolveIncludes(computeShaderSource);
	gl_id = glCreateProgram();
	std::cout << "[GlShader] Created program with id " << gl_id << std::endl;
	uint32_t cs = CompileShader(GL_COMPUTE_SHADER, computeShaderSource);
	glAttachShader(gl_id, cs);
	glLinkProgram(gl_id);
	glValidateProgram(gl_id);
	glDeleteShader(cs);
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
	}
	gl_id = -1;
}

void sf::GlShader::Bind() const
{
	assert(gl_id != -1);
	glUseProgram(gl_id);
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
