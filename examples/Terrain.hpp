#pragma once

#include <glm/glm.hpp>
#include <MeshProcessor.h>
#include <Components/TerrainCollider.h>

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
			this->vertexBufferLayout = BufferLayout({BufferComponent::Position, BufferComponent::UV});
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

			float size = (float)(this->heightmapResolution - 1) * this->heightmapPixelSize;
			TerrainCollider& tc = this->entity.AddComponent<TerrainCollider>();
			tc.aabbMin = glm::vec3(0.0f, 0.0f, 0.0f);
			tc.aabbMax = glm::vec3(size, maxHeight, size);
			tc.bitmap = &heightmap;
		}

		bool Sample(const glm::vec3 point, float& outHeight)
		{
			TerrainCollider worldSpaceTerrainCollider = EntityIntersect::WorldSpace<TerrainCollider>(entity);
			return worldSpaceTerrainCollider.Sample(point, outHeight);
		}

		void Destroy(Scene& scene)
		{
			scene.DestroyEntity(this->entity);
		}
	};
}