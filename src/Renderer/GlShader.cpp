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

	std::string GenerateVertexAttributeShaderHeader(const sf::BufferLayout& vertexBufferLayout, bool isVertexShader = true)
	{
		uint32_t currentLocation = 0;
		std::string out = "";
		for (const sf::BufferComponentInfo& bci : vertexBufferLayout.GetComponentInfos())
		{
			assert((uint32_t)bci.component < (uint32_t) sf::BufferComponent::VoxelPosition); // should be vertex component
			if (isVertexShader)
			{
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
			}
			switch (bci.component)
			{
				case sf::BufferComponent::VertexPosition:
					out += (isVertexShader ? "VA_Position;\n#define HAS_VA_Position 1\n" : "#define HAS_VA_Position 1\n"); break;
				case sf::BufferComponent::VertexNormal:
					out += (isVertexShader ? "VA_Normal;\n#define HAS_VA_Normal 1\n" : "#define HAS_VA_Normal 1\n"); break;
				case sf::BufferComponent::VertexTangent:
					out += (isVertexShader ? "VA_Tangent;\n#define HAS_VA_Tangent 1\n" : "#define HAS_VA_Tangent 1\n"); break;
				case sf::BufferComponent::VertexColor:
					out += (isVertexShader ? "VA_Color;\n#define HAS_VA_Color 1\n" : "#define HAS_VA_Color 1\n"); break;
				case sf::BufferComponent::VertexUV:
					out += (isVertexShader ? "VA_UV;\n#define HAS_VA_UV 1\n" : "#define HAS_VA_UV 1\n"); break;
				case sf::BufferComponent::VertexAO:
					out += (isVertexShader ? "VA_AO;\n#define HAS_VA_AO 1\n" : "#define HAS_VA_AO 1\n"); break;
				case sf::BufferComponent::VertexBoneWeights:
					out += (isVertexShader ? "VA_BoneWeights;\n#define HAS_VA_BoneWeights 1\n" : "#define HAS_VA_BoneWeights 1\n"); break;
				case sf::BufferComponent::VertexBoneIndices:
					out += (isVertexShader ? "VA_BoneIndices;\n#define HAS_VA_BoneIndices 1\n" : "#define HAS_VA_BoneIndices 1\n"); break;
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

	std::string GenerateParticleShaderHeader(const sf::BufferLayout& particleBufferLayout)
	{
		std::string out = "";
		for (const sf::BufferComponentInfo& bci : particleBufferLayout.GetComponentInfos())
		{
			assert(
				(uint32_t)bci.component >= (uint32_t) sf::BufferComponent::ParticlePosition); // should be particle component
			// assume buffer is only floats
			uint32_t stride = particleBufferLayout.GetSize() / 4u;
			uint32_t baseOffset = bci.byteOffset / 4u;
			switch (bci.component)
			{
			case sf::BufferComponent::ParticlePosition:
				out += "#define LOAD_PARTICLE_POSITION vec3("
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 0) + "], "
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 1) + "], "
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 2) + "])\n";
				break;
			case sf::BufferComponent::ParticleRotation:
				out += "#define LOAD_PARTICLE_ROTATION vec4("
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 0) + "], "
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 1) + "], "
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 2) + "], "
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 3) + "])\n";
				break;
			case sf::BufferComponent::ParticleScale:
				out += "#define LOAD_PARTICLE_SCALE "
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 0) + "]\n";
				break;
			case sf::BufferComponent::ParticleSpawnTime:
				out += "#define LOAD_PARTICLE_SPAWN_TIME "
					"PARTICLE_BUFFER[gl_InstanceID * " + std::to_string(stride) + " + " + std::to_string(baseOffset + 0) + "]\n";
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

void sf::GlShader::Create(const Material& material,
	const BufferLayout* vertexBufferLayout,
	const BufferLayout* voxelBufferLayout,
	const BufferLayout* particleBufferLayout)
{
	if (material.UsesMeshShader())
	{
		if (material.UsesTaskShader())
			m_taskFileName = material.taskShaderFilePath + ".glsl";
		m_meshFileName = material.meshShaderFilePath + ".glsl";
		m_fragFileName = material.fragShaderFilePath + ".glsl";
		if (material.UsesTaskShader())
			std::cout << "[GlShader] Creating shader from files: " << m_taskFileName << ", " << m_meshFileName + ", " << m_fragFileName << std::endl;
		else
			std::cout << "[GlShader] Creating shader from files: " << m_meshFileName + ", " << m_fragFileName << std::endl;
		Delete();
		std::ifstream ifs, ifs2, ifs3;
		if (material.UsesTaskShader())
			ifs = std::ifstream(m_taskFileName);
		ifs2 = std::ifstream(m_meshFileName);
		ifs3 = std::ifstream(m_fragFileName);
		if (material.UsesTaskShader())
			if (ifs.fail())
				std::cout << "[GlShader] Could not read task shader file: " << m_taskFileName << std::endl;
		if (ifs2.fail())
			std::cout << "[GlShader] Could not read mesh shader file: " << m_meshFileName << std::endl;
		if (ifs3.fail())
			std::cout << "[GlShader] Could not read fragment shader file: " << m_fragFileName << std::endl;
		std::string taskShaderSource, meshShaderSource, fragShaderSource;
		if (material.UsesTaskShader())
			taskShaderSource = std::string((std::istreambuf_iterator<char>(ifs)),
				(std::istreambuf_iterator<char>()));
		meshShaderSource = std::string((std::istreambuf_iterator<char>(ifs2)),
			(std::istreambuf_iterator<char>()));
		fragShaderSource = std::string((std::istreambuf_iterator<char>(ifs3)),
			(std::istreambuf_iterator<char>()));
		if (material.UsesTaskShader())
			taskShaderSource = "#version 460\n#extension GL_NV_mesh_shader : require\n" + taskShaderSource;
		meshShaderSource = "#version 460\n#extension GL_NV_mesh_shader : require\n" + meshShaderSource;
		fragShaderSource = "#version 460\nlayout(location = 0) out vec4 OUT_COLOR;\n" + fragShaderSource;
		if (material.UsesTaskShader())
			ResolveIncludes(taskShaderSource);
		ResolveIncludes(meshShaderSource);
		ResolveIncludes(fragShaderSource);

		gl_id = glCreateProgram();
		std::cout << "[GlShader] Created program with id " << gl_id << std::endl;
		uint32_t ts, ms, fs;
		if (material.UsesTaskShader())
			ts = CompileShader(GL_TASK_SHADER_NV, taskShaderSource);
		ms = CompileShader(GL_MESH_SHADER_NV, meshShaderSource);
		fs = CompileShader(GL_FRAGMENT_SHADER, fragShaderSource);

		if (material.UsesTaskShader())
			if (!ts)
				std::cout << taskShaderSource << std::endl;
		if (!ms)
			std::cout << meshShaderSource << std::endl;
		if (!fs)
			std::cout << fragShaderSource << std::endl;

		if (material.UsesTaskShader())
			glAttachShader(gl_id, ts);
		glAttachShader(gl_id, ms);
		glAttachShader(gl_id, fs);
		glLinkProgram(gl_id);
		CheckLinkStatusAndReturnProgram(gl_id, true);
		glValidateProgram(gl_id);

		if (material.UsesTaskShader())
			glDeleteShader(ts);
		glDeleteShader(ms);
		glDeleteShader(fs);
		return;
	}
	m_vertFileName = material.vertShaderFilePath + ".glsl";
	if (material.UsesTessellation())
	{
		m_tescFileName = material.tescShaderFilePath + ".glsl";
		m_teseFileName = material.teseShaderFilePath + ".glsl";
	}
	m_fragFileName = material.fragShaderFilePath + ".glsl";
	std::cout << "[GlShader] Creating shader from files: " << m_vertFileName << ", " << (material.UsesTessellation() ? m_tescFileName + ", " + m_teseFileName + ", " : "") << m_fragFileName << std::endl;

	Delete();

	std::ifstream ifs, ifs2, ifs3, ifs4;
	ifs = std::ifstream(m_vertFileName);
	if (material.UsesTessellation())
	{
		ifs2 = std::ifstream(m_tescFileName);
		ifs3 = std::ifstream(m_teseFileName);
	}
	ifs4 = std::ifstream(m_fragFileName);

	if (ifs.fail())
		std::cout << "[GlShader] Could not read vertex shader file: " << m_vertFileName << std::endl;
	if (material.UsesTessellation())
	{
		if (ifs2.fail())
			std::cout << "[GlShader] Could not read tessellation control shader file: " << m_tescFileName << std::endl;
		if (ifs3.fail())
			std::cout << "[GlShader] Could not read tessellation evaluation shader file: " << m_teseFileName << std::endl;
	}
	if (ifs4.fail())
		std::cout << "[GlShader] Could not read fragment shader file: " << m_fragFileName << std::endl;

	std::string vertShaderSource, tescShaderSource, teseShaderSource, fragShaderSource;
	vertShaderSource = std::string((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
	if (material.UsesTessellation())
	{
		tescShaderSource = std::string((std::istreambuf_iterator<char>(ifs2)),
			(std::istreambuf_iterator<char>()));
		teseShaderSource = std::string((std::istreambuf_iterator<char>(ifs3)),
			(std::istreambuf_iterator<char>()));
	}
	fragShaderSource = std::string((std::istreambuf_iterator<char>(ifs4)),
		(std::istreambuf_iterator<char>()));

	if (voxelBufferLayout != nullptr)
		vertShaderSource = GenerateVoxelVolumeShaderHeader(*voxelBufferLayout) + vertShaderSource;
	if (particleBufferLayout != nullptr)
		vertShaderSource = GenerateParticleShaderHeader(*particleBufferLayout) + vertShaderSource;
	vertShaderSource = GenerateVertexAttributeShaderHeader(*vertexBufferLayout) + vertShaderSource;
	vertShaderSource = "#version 460\n" + vertShaderSource;
	if (material.UsesTessellation())
	{
		tescShaderSource = GenerateVertexAttributeShaderHeader(*vertexBufferLayout, false) + tescShaderSource;
		tescShaderSource = "layout (vertices=" + std::to_string(material.tessPatchVertexCount) + ") out;\n#define VERTICES_PER_PATCH " +
			std::to_string(material.tessPatchVertexCount) + std::string("\n") + tescShaderSource;
		tescShaderSource = "#version 460\n" + tescShaderSource;
		teseShaderSource = GenerateVertexAttributeShaderHeader(*vertexBufferLayout, false) + teseShaderSource;
		teseShaderSource = "layout (" + std::string(material.tessPatchVertexCount == 3 ? "triangles" : "quads") + ", " + material.tessSpacing + ", " + material.tessWinding + ") in;\n#define VERTICES_PER_PATCH " +
			std::to_string(material.tessPatchVertexCount) + std::string("\n") + teseShaderSource;
		teseShaderSource = "#version 460\n" + teseShaderSource;
	}
	fragShaderSource = "#version 460\nlayout(location = 0) out vec4 OUT_COLOR;\n" + fragShaderSource;

	ResolveIncludes(vertShaderSource);
	if (material.UsesTessellation())
	{
		ResolveIncludes(tescShaderSource);
		ResolveIncludes(teseShaderSource);
	}
	ResolveIncludes(fragShaderSource);

	gl_id = glCreateProgram();
	std::cout << "[GlShader] Created program with id " << gl_id << std::endl;
	uint32_t vs, tcs, tes, fs;
	vs = CompileShader(GL_VERTEX_SHADER, vertShaderSource);
	if (material.UsesTessellation())
	{
		tcs = CompileShader(GL_TESS_CONTROL_SHADER, tescShaderSource);
		tes = CompileShader(GL_TESS_EVALUATION_SHADER, teseShaderSource);
	}
	fs = CompileShader(GL_FRAGMENT_SHADER, fragShaderSource);

	if (!vs)
		std::cout << vertShaderSource << std::endl;
	if (material.UsesTessellation())
	{
		if (!tcs)
			std::cout << tescShaderSource << std::endl;
		if (!tes)
			std::cout << teseShaderSource << std::endl;
	}
	if (!fs)
		std::cout << fragShaderSource << std::endl;

	glAttachShader(gl_id, vs);
	if (material.UsesTessellation())
	{
		glAttachShader(gl_id, tcs);
		glAttachShader(gl_id, tes);
	}
	glAttachShader(gl_id, fs);
	glLinkProgram(gl_id);
	CheckLinkStatusAndReturnProgram(gl_id, true);
	glValidateProgram(gl_id);

	glDeleteShader(vs);
	if (material.UsesTessellation())
	{
		glDeleteShader(tcs);
		glDeleteShader(tes);
	}
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
		std::cout << "[GlShader] Could not get uniform location for '" << name << "' in shader " << m_vertFileName << "-" << m_fragFileName << std::endl;

	m_uniformCache[name].location = location;
	return location;
}

int sf::GlShader::GetOrAssignTextureIndex(const std::string& name)
{
	if (m_uniformCache[name].textureIndex != -1)
	{
		return m_uniformCache[name].textureIndex;
	}
	m_uniformCache[name].textureIndex = m_textureIndexCounter;
	m_textureIndexCounter++;
	return m_textureIndexCounter - 1;
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
void sf::GlShader::SetUniform1iv(const std::string& name, const int32_t* pointer, uint32_t number)
{
	glUniform1iv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform2iv(const std::string& name, const int32_t* pointer, uint32_t number)
{
	glUniform2iv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform3iv(const std::string& name, const int32_t* pointer, uint32_t number)
{
	glUniform3iv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform4iv(const std::string& name, const int32_t* pointer, uint32_t number)
{
	glUniform4iv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform1uiv(const std::string& name, const uint32_t* pointer, uint32_t number)
{
	glUniform1uiv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform2uiv(const std::string& name, const uint32_t* pointer, uint32_t number)
{
	glUniform2uiv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform3uiv(const std::string& name, const uint32_t* pointer, uint32_t number)
{
	glUniform3uiv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform4uiv(const std::string& name, const uint32_t* pointer, uint32_t number)
{
	glUniform4uiv(GetUniformLocation(name), number, pointer);
}
void sf::GlShader::SetUniform1f(const std::string& name, float value)
{
	glUniform1f(GetUniformLocation(name), value);
}
void sf::GlShader::SetUniform1i(const std::string& name, int32_t value)
{
	glUniform1i(GetUniformLocation(name), value);
}
void sf::GlShader::SetUniform1u(const std::string& name, uint32_t value)
{
	glUniform1ui(GetUniformLocation(name), value);
}