#include "IblHelper.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image_write.h>
#include <iostream>

#include <Config.h>
#include <Renderer/GlShader.h>
#include <Bitmap.h>

namespace sf::IblHelper
{
	struct Texture
	{
		uint32_t id;
		uint32_t width;
		uint32_t height;
		uint32_t levels;
	};

	uint32_t numMipmapLevels(uint32_t width, uint32_t height)
	{
		uint32_t levels = 1;
		while ((width | height) >> levels)
			++levels;
		return levels;
	}

	Texture createTexture(GLenum target, int width, int height, GLenum internalFormat, bool mipmap)
	{
		Texture texture;
		texture.width = width;
		texture.height = height;
		texture.levels = mipmap ? numMipmapLevels(width, height) : 1;

		glCreateTextures(target, 1, &texture.id);
		glTextureStorage2D(texture.id, texture.levels, internalFormat, width, height);
		glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, texture.levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		return texture;
	}

	void GenerateLUT(GlTexture& lut, DataType dataType)
	{
		assert(dataType == DataType::f16 || dataType == DataType::f32);
		int internalFormat = dataType == DataType::f16 ? GL_RG16F : GL_RG32F;
		GlTexture::StorageType textureStorageType = dataType == DataType::f16 ? GlTexture::StorageType::Float16 : GlTexture::StorageType::Float32;

		GlShader lutComputeShader;
		lutComputeShader.CreateComputeFromFile("assets/shaders/ibl/spbrdfC.glsl");

		static constexpr int kBRDF_LUT_Size = 256;
		Texture m_spBRDF_LUT = createTexture(GL_TEXTURE_2D, kBRDF_LUT_Size, kBRDF_LUT_Size, internalFormat, false);
		glTextureParameteri(m_spBRDF_LUT.id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_spBRDF_LUT.id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		lutComputeShader.Bind();
		glBindImageTexture(0, m_spBRDF_LUT.id, 0, GL_FALSE, 0, GL_WRITE_ONLY, internalFormat);
		glDispatchCompute(m_spBRDF_LUT.width / 32, m_spBRDF_LUT.height / 32, 1);

		lut.width = m_spBRDF_LUT.width;
		lut.height = m_spBRDF_LUT.height;
		lut.channelCount = 2;
		lut.storageType = textureStorageType;
		if (lut.isInitialized)
			glDeleteTextures(1, &lut.gl_id);
		lut.gl_id = m_spBRDF_LUT.id;
		lut.isInitialized = true;
	}

	void CubemapFromHdr(const std::string& hdrFilePath, GlCubemap& environmentCubemap, DataType dataType)
	{
		assert(dataType == DataType::f16 || dataType == DataType::f32);
		int internalFormat = dataType == DataType::f16 ? GL_RGBA16F : GL_RGBA32F;
		GlCubemap::StorageType cubemapStorageType = dataType == DataType::f16 ? GlCubemap::StorageType::Float16 : GlCubemap::StorageType::Float32;

		Bitmap equirectBitmap(hdrFilePath, true, dataType == DataType::f16);
		GlTexture equirectTexture;
		equirectTexture.CreateFromBitmap(equirectBitmap, GlTexture::ClampToEdge, true, internalFormat);

		static constexpr int kEnvMapSize = 1024;
		GlShader equirect2CubeComputeShader;
		equirect2CubeComputeShader.CreateComputeFromFile("assets/shaders/ibl/equirect2cubeC.glsl");

		equirect2CubeComputeShader.Bind();
		Texture envTextureUnfiltered = createTexture(GL_TEXTURE_CUBE_MAP, kEnvMapSize, kEnvMapSize, internalFormat, true);
		glBindTextureUnit(0, equirectTexture.gl_id);
		glBindImageTexture(0, envTextureUnfiltered.id, 0, GL_TRUE, 0, GL_WRITE_ONLY, internalFormat);
		glDispatchCompute(envTextureUnfiltered.width / 32, envTextureUnfiltered.height / 32, 6);

		glGenerateTextureMipmap(envTextureUnfiltered.id);
		environmentCubemap.size = envTextureUnfiltered.width;
		environmentCubemap.storageType = cubemapStorageType;
		if (environmentCubemap.isInitialized)
			glDeleteTextures(1, &environmentCubemap.gl_id);
		environmentCubemap.gl_id = envTextureUnfiltered.id;
		environmentCubemap.isInitialized = true;
	}
	void SpecularFromEnv(const GlCubemap& environmentCubemap, GlCubemap& prefilterCubemap, DataType dataType)
	{
		assert(dataType == DataType::f16 || dataType == DataType::f32);
		int internalFormat = dataType == DataType::f16 ? GL_RGBA16F : GL_RGBA32F;
		GlCubemap::StorageType cubemapStorageType = dataType == DataType::f16 ? GlCubemap::StorageType::Float16 : GlCubemap::StorageType::Float32;

		GlShader spmapComputeShader;
		spmapComputeShader.CreateComputeFromFile("assets/shaders/ibl/spmapC.glsl");

		static constexpr int kEnvMapSize = 1024;
		Texture m_envTexture = createTexture(GL_TEXTURE_CUBE_MAP, kEnvMapSize, kEnvMapSize, internalFormat, true);

		// Copy 0th mipmap level into destination environment map.
		glCopyImageSubData(environmentCubemap.gl_id, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
			m_envTexture.id, GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
			m_envTexture.width, m_envTexture.height, 6);

		spmapComputeShader.Bind();
		glBindTextureUnit(0, environmentCubemap.gl_id);

		// Pre-filter rest of the mip chain.
		const float deltaRoughness = 1.0f / glm::max(float(m_envTexture.levels - 1), 1.0f);
		for (int level = 1, size = kEnvMapSize / 2; level <= m_envTexture.levels; ++level, size /= 2)
		{
			const GLuint numGroups = glm::max(1, size / 32);
			glBindImageTexture(0, m_envTexture.id, level, GL_TRUE, 0, GL_WRITE_ONLY, internalFormat);
			glProgramUniform1f(spmapComputeShader.gl_id, 0, level * deltaRoughness);
			glDispatchCompute(numGroups, numGroups, 6);
		}
		prefilterCubemap.size = m_envTexture.width;
		prefilterCubemap.storageType = cubemapStorageType;
		if (prefilterCubemap.isInitialized)
			glDeleteTextures(1, &prefilterCubemap.gl_id);
		prefilterCubemap.gl_id = m_envTexture.id;
		prefilterCubemap.isInitialized = true;
	}
	void IrradianceFromEnv(const GlCubemap& environmentCubemap, GlCubemap& irradianceCubemap, DataType dataType)
	{
		assert(dataType == DataType::f16 || dataType == DataType::f32);
		int internalFormat = dataType == DataType::f16 ? GL_RGBA16F : GL_RGBA32F;
		GlCubemap::StorageType cubemapStorageType = dataType == DataType::f16 ? GlCubemap::StorageType::Float16 : GlCubemap::StorageType::Float32;

		static constexpr int kIrradianceMapSize = 32;
		GlShader irmapComputeShader;
		irmapComputeShader.CreateComputeFromFile("assets/shaders/ibl/irmapC.glsl");

		Texture m_irmapTexture = createTexture(GL_TEXTURE_CUBE_MAP, kIrradianceMapSize, kIrradianceMapSize, internalFormat, false);

		irmapComputeShader.Bind();
		glBindTextureUnit(0, environmentCubemap.gl_id);
		glBindImageTexture(0, m_irmapTexture.id, 0, GL_TRUE, 0, GL_WRITE_ONLY, internalFormat);
		glDispatchCompute(m_irmapTexture.width / 32, m_irmapTexture.height / 32, 6);
		irradianceCubemap.size = m_irmapTexture.width;
		irradianceCubemap.storageType = cubemapStorageType;
		if (irradianceCubemap.isInitialized)
			glDeleteTextures(1, &irradianceCubemap.gl_id);
		irradianceCubemap.gl_id = m_irmapTexture.id;
		irradianceCubemap.isInitialized = true;
	}
}