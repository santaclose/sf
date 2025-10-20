#pragma once

#include <glm/glm.hpp>
#include <MeshProcessor.h>

namespace sf
{
	struct Terrain
	{
		BufferLayout vertexBufferLayout;
		Bitmap heightmap;
		glm::vec3 origin;
		Material material;
		Entity entity;
		MeshData mesh;
		uint32_t heightmapResolution;
		float heightmapPixelSize;
		float maxHeight;

		void Create(Scene& scene, const std::string& heightmapFilePath, float heightmapPixelSize, float maxHeight, uint32_t heightmapPixelsPerPatch, const glm::vec3& origin)
		{
			this->vertexBufferLayout = BufferLayout({BufferComponent::VertexPosition, BufferComponent::VertexUV});
			this->origin = origin;
			this->maxHeight = maxHeight;
			this->heightmapPixelSize = heightmapPixelSize;
			this->heightmap.CreateFromFile(heightmapFilePath);
			assert(this->heightmap.width == this->heightmap.height);
			this->heightmapResolution = this->heightmap.width;
			uint32_t patchCount = this->heightmapResolution / heightmapPixelsPerPatch;

			this->material.vertShaderFilePath = "assets/shaders/terrain.vert";
			this->material.tescShaderFilePath = "assets/shaders/terrain.tesc";
			this->material.teseShaderFilePath = "assets/shaders/terrain.tese";
			this->material.fragShaderFilePath = "assets/shaders/terrain.frag";
			this->material.tessSpacing = "equal_spacing";
			this->material.tessWinding = "ccw";
			this->material.tessPatchVertexCount = 4;
			this->material.uniforms["heightmapTexture"].dataType = DataType::bitmap;
			this->material.uniforms["heightmapTexture"].data.p = &this->heightmap;
			// this->material.uniforms["heightmapRes"].dataType = DataType::u32;
			// this->material.uniforms["heightmapRes"].data.u32 = this->heightmapResolution;
			this->material.uniforms["maxHeight"].dataType = DataType::f32;
			this->material.uniforms["maxHeight"].data.f32 = this->maxHeight;
			this->material.drawMode = MaterialDrawMode::Lines;

			this->mesh.vertexBufferLayout = &this->vertexBufferLayout;
			MeshProcessor::GenerateGrid(this->mesh, patchCount + 1, patchCount + 1, this->heightmapResolution, this->heightmapResolution, heightmapPixelSize * ((float)(this->heightmapResolution - 1) / (float)patchCount), true);

			this->entity = scene.CreateEntity();
			Transform& e_t = this->entity.AddComponent<Transform>();
			e_t.position = this->origin;
			this->entity.AddComponent<Mesh>(&this->mesh, &this->material);
		}

		bool Sample(const glm::vec3 point, float& outHeight)
		{
			float size = (float)(this->heightmapResolution - 1) * this->heightmapPixelSize;
			glm::vec2 heightmapUV;
			heightmapUV.x = (point.x - this->origin.x) / size;
			heightmapUV.y = -(point.z - this->origin.z) / size;
			heightmapUV.x = heightmapUV.x * ((float)(this->heightmapResolution - 1) / (float)this->heightmapResolution) + 0.5f / (float)this->heightmapResolution;
			heightmapUV.y = heightmapUV.y * ((float)(this->heightmapResolution - 1) / (float)this->heightmapResolution) + 0.5f / (float)this->heightmapResolution;
			if (heightmapUV.x > 0.0f && heightmapUV.x < 1.0f && heightmapUV.y > 0.0f && heightmapUV.y < 1.0f)
			{
				float heightSample = this->heightmap.Sample<uint16_t>(heightmapUV, 0);
				outHeight = heightSample * this->maxHeight;
				return true;
			}
			return false;
		}

		void Destroy(Scene& scene)
		{
			scene.DestroyEntity(this->entity);
		}
	};
}