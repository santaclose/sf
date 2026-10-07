#include "DebugDraw.h"

#include <Renderer/GlShader.h>

namespace sf::Renderer::Debug {

	Config config;
	glm::vec3 colliderColor = { 0.0f, 0.0f, 0.0f };
	glm::vec3 frustumCullingSphereColor = { 0.0f, 1.0f, 0.0f };

	struct LineVertex {
		glm::vec3 pos;
		glm::vec3 color;
	};
	std::vector<LineVertex> lineVector;
	bool lineDataInitialized = false;
	uint32_t lineVBO, lineVAO;
	GlShader lineShader;
	Buffer::Layout positionColorVertexLayout = Buffer::Layout({
		Buffer::Component::Position,
		Buffer::Component::Color
	});
}

void sf::Renderer::Debug::AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color)
{
	lineVector.resize(lineVector.size() + 2);
	lineVector[lineVector.size() - 2] = { a, color };
	lineVector[lineVector.size() - 1] = { b, color };
}

void sf::Renderer::Debug::AddSphereCollider(const SphereCollider& sphereCollider, const Transform& transform, const glm::vec3* color)
{
	const glm::vec3& ccolor = color == nullptr ? colliderColor : *color;
	SphereCollider sc = sphereCollider.ApplyTransform(transform);
	AddLine(sc.center + glm::vec3(sc.radius, 0.0f, 0.0f), sc.center - glm::vec3(sc.radius, 0.0f, 0.0f), ccolor);
	AddLine(sc.center + glm::vec3(0.0f, sc.radius, 0.0f), sc.center - glm::vec3(0.0f, sc.radius, 0.0f), ccolor);
	AddLine(sc.center + glm::vec3(0.0f, 0.0f, sc.radius), sc.center - glm::vec3(0.0f, 0.0f, sc.radius), ccolor);
}
void sf::Renderer::Debug::AddCapsuleCollider(const CapsuleCollider& capsuleCollider, const Transform& transform, const glm::vec3* color)
{
	const glm::vec3& ccolor = color == nullptr ? colliderColor : *color;
	CapsuleCollider cc = capsuleCollider.ApplyTransform(transform);
	AddLine(cc.centerA + glm::vec3(cc.radius, 0.0f, 0.0f), cc.centerA - glm::vec3(cc.radius, 0.0f, 0.0f), ccolor);
	AddLine(cc.centerA + glm::vec3(0.0f, cc.radius, 0.0f), cc.centerA - glm::vec3(0.0f, cc.radius, 0.0f), ccolor);
	AddLine(cc.centerA + glm::vec3(0.0f, 0.0f, cc.radius), cc.centerA - glm::vec3(0.0f, 0.0f, cc.radius), ccolor);
	AddLine(cc.centerB + glm::vec3(cc.radius, 0.0f, 0.0f), cc.centerB - glm::vec3(cc.radius, 0.0f, 0.0f), ccolor);
	AddLine(cc.centerB + glm::vec3(0.0f, cc.radius, 0.0f), cc.centerB - glm::vec3(0.0f, cc.radius, 0.0f), ccolor);
	AddLine(cc.centerB + glm::vec3(0.0f, 0.0f, cc.radius), cc.centerB - glm::vec3(0.0f, 0.0f, cc.radius), ccolor);
	AddLine(cc.centerA, cc.centerB, ccolor);
}
void sf::Renderer::Debug::AddBoxCollider(const BoxCollider& boxCollider, const Transform& transform, const glm::vec3* color)
{
	const glm::vec3& ccolor = color == nullptr ? colliderColor : *color;
	BoxCollider bc = boxCollider.ApplyTransform(transform);
	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), ccolor);

	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), ccolor);

	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), ccolor);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), ccolor);
}
void sf::Renderer::Debug::AddMeshCollider(const MeshCollider& meshCollider, const Transform& transform, const glm::vec3* color)
{
	AddSphereCollider(meshCollider.boundingSphere, transform, color);
}

void sf::Renderer::Debug::AddSkeleton(const SkinnedMesh& mesh, const Transform& transform)
{
	uint32_t boneCount = mesh.skeletonData->GetBoneCount();
	for (uint32_t i = 0; i < boneCount; i++)
	{
		const BoneData& currentBone = mesh.skeletonData->GetBoneData(i);
		Transform parentT = transform;
		if (currentBone.parent >= 0)
			parentT.Apply(mesh.skeletonData->GetBoneTransform(currentBone.parent));
		Transform childT = transform;
		childT.Apply(mesh.skeletonData->GetBoneTransform(i));
		float boneRatio = (float) i / (float) boneCount;
		AddLine(parentT.position, childT.position, glm::vec3(0.0f, 1.0f - boneRatio, boneRatio));
	}
}

void sf::Renderer::Debug::AddSkeletonModelSpace(const SkinnedMesh& mesh, const Transform& transform)
{
	uint32_t boneCount = mesh.skeletonData->GetBoneCount();
	for (uint32_t i = 0; i < boneCount; i++)
	{
		const BoneData& currentBone = mesh.skeletonData->GetBoneData(i);
		Transform parentT = transform;
		if (currentBone.parent >= 0)
			parentT.Apply(mesh.skeletonData->GetInverseModelTransform(currentBone.parent).Inverse());
		Transform childT = transform;
		childT.Apply(currentBone.invModelTransform.Inverse());
		float boneRatio = (float) i / (float) boneCount;
		AddLine(parentT.position, childT.position, glm::vec3(0.0f, 1.0f - boneRatio, boneRatio));
	}
}

void sf::Renderer::Debug::AddFrustumCullingSphere(const SphereCollider& sphereCollider)
{
	const SphereCollider& sc = sphereCollider;
	AddLine(sc.center + glm::vec3(sc.radius, 0.0f, 0.0f), sc.center - glm::vec3(sc.radius, 0.0f, 0.0f), frustumCullingSphereColor);
	AddLine(sc.center + glm::vec3(0.0f, sc.radius, 0.0f), sc.center - glm::vec3(0.0f, sc.radius, 0.0f), frustumCullingSphereColor);
	AddLine(sc.center + glm::vec3(0.0f, 0.0f, sc.radius), sc.center - glm::vec3(0.0f, 0.0f, sc.radius), frustumCullingSphereColor);
}

void sf::Renderer::Debug::Draw(uint32_t sharedGpuDataUbo)
{
	if (lineVector.size() == 0)
		return;
	glDisable(GL_DEPTH_TEST);
	if (!lineDataInitialized)
	{
		glGenVertexArrays(1, &lineVAO);
		glGenBuffers(1, &lineVBO);

		glBindVertexArray(lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, lineVector.size() * sizeof(LineVertex), lineVector.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));

		Material lineMat;
		lineMat.vertShaderFilePath = "assets/shaders/drawLines.vert";
		lineMat.fragShaderFilePath = "assets/shaders/drawLines.frag";
		lineShader.Create(lineMat, positionColorVertexLayout);

		lineDataInitialized = true;
	}

	lineShader.Bind();
	glPolygonMode(GL_FRONT, GL_FILL);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuDataUbo);

	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

	glBufferData(GL_ARRAY_BUFFER, lineVector.size() * sizeof(LineVertex), lineVector.data(), GL_DYNAMIC_DRAW);

	glDrawArrays(GL_LINES, 0, lineVector.size());

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void sf::Renderer::Debug::FrameEnd()
{
	lineVector.clear();
}

void sf::Renderer::Debug::Terminate()
{
	if (lineDataInitialized)
	{
		lineShader.Delete();
		glDeleteBuffers(1, &lineVBO);
		glDeleteVertexArrays(1, &lineVAO);
	}
}