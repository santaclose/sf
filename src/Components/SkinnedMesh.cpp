#include "SkinnedMesh.h"

void sf::ApplySkeleton(const glm::mat4* skinMatrixBuffer, const MeshData& inputMesh, MeshData& outputMesh)
{
	assert(inputMesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::BoneIndices) != nullptr);
	assert(inputMesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::BoneWeights) != nullptr);
	assert(outputMesh.vertexBufferLayout.GetComponentInfo(Buffer::Component::Position) != nullptr);

	for (uint32_t i = 0; i < inputMesh.vertexCount; i++)
	{
		const glm::vec4* indices = inputMesh.ReadVertexComponent<glm::vec4>(Buffer::Component::BoneIndices, i);
		const glm::vec4* weights = inputMesh.ReadVertexComponent<glm::vec4>(Buffer::Component::BoneWeights, i);

		const glm::mat4* matA = &skinMatrixBuffer[(uint32_t) indices->x];
		const glm::mat4* matB = &skinMatrixBuffer[(uint32_t) indices->y];
		const glm::mat4* matC = &skinMatrixBuffer[(uint32_t) indices->z];
		const glm::mat4* matD = &skinMatrixBuffer[(uint32_t) indices->w];

		glm::mat4 skinMatrix = weights->x * *matA + weights->y * *matB + weights->z * *matC + weights->w * *matD;

		const glm::vec3* inPos = inputMesh.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, i);
		glm::vec3* outPos = outputMesh.AccessVertexComponent<glm::vec3>(Buffer::Component::Position, i);

		*outPos = skinMatrix * glm::vec4(inPos->x, inPos->y, inPos->z, 1.0f);
	}
}

void sf::SkinnedMesh::ComputeBoundingGeometry()
{
	assert(skeletonData != nullptr);
	const MeshData* sourceMeshData = GetSourceMeshData();
	static const Buffer::Layout positionOnlyVertexLayout = Buffer::Layout({
		Buffer::Component::Position
	});
	MeshData tempM;
	tempM.vertexBufferLayout = positionOnlyVertexLayout;
	tempM.vertexCount = sourceMeshData->vertexCount;
	tempM.vertexBuffer = new uint8_t[sizeof(glm::vec3) * sourceMeshData->vertexCount];
	SkeletonCoreData tempS = skeletonData->m_core;
	const std::vector<Animation::SkeletalAnimation>& animations = skeletonData->m_animations;

	/* Get keyframes */
	std::vector<std::unordered_set<float>> allKeyFrameTimesPerAnimation(animations.size());
	for (uint32_t i = 0; i < animations.size(); i++)
	{
		const Animation::SkeletalAnimation& anim = animations[i];
		for (const Animation::Channel& channel : anim.channels)
		{
			const Animation::Sampler& sampler = anim.samplers[channel.samplerIndex];
			if (sampler.inputs.size() > sampler.outputsVec4.size())
				continue;

			for (float samplerInput : sampler.inputs)
				allKeyFrameTimesPerAnimation[i].insert(samplerInput);
		}
	}

	/* Bounding box containing every animation */
	boundingAABB.min = glm::vec3(std::numeric_limits<float>::max());
	boundingAABB.max = glm::vec3(-std::numeric_limits<float>::max());
	for (uint32_t i = 0; i < animations.size(); i++)
	{
		const Animation::SkeletalAnimation& anim = animations[i];
		for (float kf : allKeyFrameTimesPerAnimation[i])
		{
			Animation::SampleToBones(anim, tempS.m_boneLocalTransforms.data(), kf);
			tempS.PropagateLocalTransforms(nullptr, true);

			ApplySkeleton(tempS.m_skinningMatrices.data(), *sourceMeshData, tempM);
			for (uint32_t vi = 0; vi < tempM.vertexCount; vi++)
			{
				const glm::vec3* vertexPos = tempM.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, vi);
				boundingAABB.min = glm::min(boundingAABB.min, *vertexPos);
				boundingAABB.max = glm::max(boundingAABB.max, *vertexPos);
			}
		}
	}

	/* Bounding sphere containing every animation */
	boundingSphere.center = (boundingAABB.min + boundingAABB.max) * 0.5f;
	float maxDistanceSq = 0.0f;
	for (uint32_t i = 0; i < animations.size(); i++)
	{
		const Animation::SkeletalAnimation& anim = animations[i];
		for (float kf : allKeyFrameTimesPerAnimation[i])
		{
			Animation::SampleToBones(anim, tempS.m_boneLocalTransforms.data(), kf);
			tempS.PropagateLocalTransforms(nullptr, true);

			ApplySkeleton(tempS.m_skinningMatrices.data(), *sourceMeshData, tempM);
			for (uint32_t vi = 0; vi < tempM.vertexCount; vi++)
			{
				const glm::vec3* vertexPos = tempM.ReadVertexComponent<glm::vec3>(Buffer::Component::Position, vi);
				float distSq = glm::distance2(boundingSphere.center, *vertexPos);
				if (distSq > maxDistanceSq)
					maxDistanceSq = distSq;
			}
		}
	}
	boundingSphere.radius = glm::sqrt(maxDistanceSq);
	tempM.Destroy();
}