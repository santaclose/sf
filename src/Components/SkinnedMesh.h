#pragma once

#include <Material.h>
#include <LOD.h>
#include <SkeletonData.h>
#include <vector>
#include <unordered_map>

#include <Components/SphereCollider.h>
#include <Geometry/AABB.h>

namespace sf {

	struct SkinnedMesh
	{
		const LODGroup* lodGroup = nullptr;
		const SkeletonData* skeletonData = nullptr;
		std::vector<const Material*> materials;
		std::vector<std::unordered_map<std::string, UniformData>> uniformOverrides;
		bool usesLodGroup;

		AABB boundingAABB;
		SphereCollider boundingSphere;

		inline SkinnedMesh(const LODGroup* lodGroup, const SkeletonData* skeletonData, const Material* material = nullptr)
		{
			assert(lodGroup != nullptr);
			assert(lodGroup->lods.size() > 0);
			this->lodGroup = lodGroup;
			this->materials.clear();
			this->materials.resize(lodGroup->sourceMesh->pieceCount, material);
			this->uniformOverrides.resize(lodGroup->sourceMesh->pieceCount);
			this->skeletonData = skeletonData;
			usesLodGroup = true;
			ComputeBoundingGeometry();
		}
		inline SkinnedMesh(const MeshData* meshData, const SkeletonData* skeletonData, const Material* material = nullptr)
		{
			/* Use the lodGroup pointer just as storage */
			this->lodGroup = (LODGroup*) meshData;
			this->materials.clear();
			this->materials.resize(meshData->pieceCount, material);
			this->uniformOverrides.resize(meshData->pieceCount);
			this->skeletonData = skeletonData;
			usesLodGroup = false;
			ComputeBoundingGeometry();
		}

		inline const MeshData* GetSourceMeshData() const
		{
			if (!usesLodGroup)
				return (const MeshData*) lodGroup;
			return lodGroup->sourceMesh;
		}

		void ComputeBoundingGeometry();

		const AABB& GetBoundingAABB() const
		{
			return boundingAABB;
		}

		const SphereCollider& GetBoundingSphere() const
		{
			return boundingSphere;
		}

		inline const MeshData* GetMeshData(float camDistance2) const
		{
			if (!usesLodGroup)
				return (const MeshData*) lodGroup;

			for (uint32_t i = 0; i < lodGroup->GetLODCount(); i++)
			{
				const LOD& lod = lodGroup->lods[i];
				if (camDistance2 < lod.maxDistance * lod.maxDistance)
					return lodGroup->GetMeshDataForLOD(i);
			}
			return lodGroup->GetMeshDataForLOD(lodGroup->GetLODCount() - 1);
		}

		inline uint32_t GetCurrentLOD(float camDistance2) const
		{
			assert(usesLodGroup);
			for (uint32_t i = 0; i < lodGroup->GetLODCount(); i++)
			{
				const LOD& lod = lodGroup->lods[i];
				if (camDistance2 < lod.maxDistance * lod.maxDistance)
					return i;
			}
			return lodGroup->GetLODCount() - 1;
		}
	};

	void ApplySkeleton(const glm::mat4* skinMatrixBuffer, const MeshData& inputMesh, MeshData& outputMesh);
}