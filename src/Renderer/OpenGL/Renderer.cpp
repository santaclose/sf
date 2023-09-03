#ifdef SF_USE_OPENGL

#include "../Renderer.h"

#include <glad/glad.h>
#include <assert.h>
#include <iostream>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Config.h>
#include <Bitmap.h>

#include <Scene/Entity.h>
#include <Components/Camera.h>

#include <Material.h>
#include <Defaults.h>

#include <Renderer/OpenGL/GlMaterial.h>
#include <Renderer/OpenGL/GlSkybox.h>
#include <Renderer/OpenGL/IblHelper.h>

namespace sf::Renderer
{
	const Window* window;

	GlShader defaultShader;
	GlMaterial defaultMaterial;
	GlShader defaultSkinningShader;
	GlMaterial defaultSkinningMaterial;
	GlShader voxelBoxShader;

	float aspectRatio = 1.7777777777;
	Entity activeCameraEntity;
	bool drawSkybox = false;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;

	GlShader spriteShader;

	struct MeshGpuData
	{
		uint32_t gl_vertexBuffer;
		uint32_t gl_indexBuffer;
		uint32_t gl_vao;
	};

	struct VoxelBoxGpuData
	{
		uint32_t gl_ssbo;
		int numberOfCubes;
		std::vector<glm::mat4> cubeModelMatrices;
	};

	struct SpriteQuad
	{
		uint32_t gl_vertexBuffer;
		uint32_t gl_indexBuffer;
		uint32_t gl_vao;
		glm::vec2 vertices[8];
		uint32_t indices[6];
	};
	SpriteQuad spriteQuad;
	std::unordered_map<const sf::Bitmap*, GlTexture> spriteTextures;

	struct SharedGpuData
	{
		glm::mat4 modelMatrix;
		glm::mat4 cameraMatrix;
		glm::mat4 screenSpaceMatrix;
		glm::vec3 cameraPosition;
	};
	uint32_t sharedGpuData_gl_ubo;
	SharedGpuData sharedGpuData;

	std::unordered_map<const sf::SkeletonData*, uint32_t> skeletonSsbos;

	std::unordered_map<const sf::MeshData*, MeshGpuData> meshGpuData;
	std::unordered_map<int, std::vector<GlMaterial*>> meshMaterials;

	std::unordered_map<const sf::VoxelBoxData*, VoxelBoxGpuData> voxelBoxGpuData;

	std::vector<GlMaterial*> materials;
	struct EnvironmentData
	{
		GlTexture envTexture;
		GlCubemap envCubemap;
		GlCubemap irradianceCubemap;
		GlCubemap prefilterCubemap;
		GlTexture lookupTexture;
	};
	std::vector<void*> rendererUniformVector;
	EnvironmentData environmentData;
	uint32_t environment_gl_ubo;

	void CreateMeshMaterialSlots(int id, const sf::MeshData* mesh)
	{
		for (int i = 0; i < mesh->pieces.size(); i++)
			meshMaterials[id].push_back(&defaultMaterial);
	}
	void CreateSkinnedMeshMaterialSlots(int id, const sf::MeshData* mesh)
	{
		for (int i = 0; i < mesh->pieces.size(); i++)
			meshMaterials[id].push_back(&defaultSkinningMaterial);
	}

	void TransferVertexData(const sf::MeshData* mesh)
	{
		glBindBuffer(GL_ARRAY_BUFFER, meshGpuData[mesh].gl_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * mesh->vertexLayout.GetSize(), mesh->vertexBuffer, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshGpuData[mesh].gl_indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexVector.size() * sizeof(uint32_t), &mesh->indexVector[0], GL_STATIC_DRAW);
	}

	void CreateMeshGpuData(const sf::MeshData* mesh)
	{
		meshGpuData[mesh] = MeshGpuData();
		glGenVertexArrays(1, &meshGpuData[mesh].gl_vao);
		glGenBuffers(1, &meshGpuData[mesh].gl_vertexBuffer);
		glGenBuffers(1, &meshGpuData[mesh].gl_indexBuffer);

		glBindVertexArray(meshGpuData[mesh].gl_vao);
		glBindBuffer(GL_ARRAY_BUFFER, meshGpuData[mesh].gl_vertexBuffer);

		// update vertices
		glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * mesh->vertexLayout.GetSize(), mesh->vertexBuffer, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshGpuData[mesh].gl_indexBuffer);
		// update indices to draw
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexVector.size() * sizeof(uint32_t), &mesh->indexVector[0], GL_STATIC_DRAW);

		const std::vector<DataComponent>& components = mesh->vertexLayout.GetComponents();
		for (int i = 0; i < components.size(); i++)
		{
			glEnableVertexAttribArray(i);
			switch (components[i].dataType)
			{
				case DataType::f32:
					glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, mesh->vertexLayout.GetSize(), (void*)components[i].byteOffset);
					break;
				case DataType::vec2f32:
					glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, mesh->vertexLayout.GetSize(), (void*)components[i].byteOffset);
					break;
				case DataType::vec3f32:
					glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, mesh->vertexLayout.GetSize(), (void*)components[i].byteOffset);
					break;
				case DataType::vec4f32:
					glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, mesh->vertexLayout.GetSize(), (void*)components[i].byteOffset);
					break;
				case DataType::vec4u8:
					glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_FALSE, mesh->vertexLayout.GetSize(), (void*)components[i].byteOffset);
					break;
				case DataType::vec4u16:
					glVertexAttribPointer(i, 4, GL_UNSIGNED_SHORT, GL_FALSE, mesh->vertexLayout.GetSize(), (void*)components[i].byteOffset);
					break;
			}
		}

		TransferVertexData(mesh);
		glBindVertexArray(0);
	}

	void CreateVoxelBoxGpuData(const sf::VoxelBoxData* voxelBox, const Transform& transform)
	{
		voxelBoxGpuData[voxelBox] = VoxelBoxGpuData();

		Transform voxelSpaceCursor;
		voxelSpaceCursor.scale = voxelBox->voxelSize;

		int currentCube = 0;
		for (int i = 0; i < voxelBox->mat.size(); i++)
		{
			for (int j = 0; j < voxelBox->mat[0].size(); j++)
			{
				for (int k = 0; k < voxelBox->mat[0][0].size(); k++)
				{
					if (voxelBox->mat[i][j][k])
					{
						voxelSpaceCursor.position = voxelBox->offset;
						voxelSpaceCursor.position.x += voxelBox->voxelSize * ((float)i + 0.5f);
						voxelSpaceCursor.position.y += voxelBox->voxelSize * ((float)j + 0.5f);
						voxelSpaceCursor.position.z += voxelBox->voxelSize * ((float)k + 0.5f);

						voxelBoxGpuData[voxelBox].cubeModelMatrices.emplace_back();
						voxelBoxGpuData[voxelBox].cubeModelMatrices[currentCube] = voxelSpaceCursor.ComputeMatrix();
						currentCube++;
					}
				}
			}
		}
		voxelBoxGpuData[voxelBox].numberOfCubes = currentCube;

		glGenBuffers(1, &(voxelBoxGpuData[voxelBox].gl_ssbo));
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxelBoxGpuData[voxelBox].gl_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, voxelBoxGpuData[voxelBox].cubeModelMatrices.size() * sizeof(glm::mat4), &(voxelBoxGpuData[voxelBox].cubeModelMatrices[0][0][0]), GL_STATIC_DRAW);

		return;
	}

	void SetMeshMaterial(const Mesh& mesh, GlMaterial* material, int piece = -1)
	{
		if (meshGpuData.find(mesh.meshData) == meshGpuData.end()) // create mesh data if not there
			CreateMeshGpuData(mesh.meshData);
		if (meshMaterials.find(mesh.id) == meshMaterials.end())
			CreateMeshMaterialSlots(mesh.id, mesh.meshData);

		if (piece < 0) // set for all pieces by default
		{
			for (int i = 0; i < meshMaterials[mesh.id].size(); i++)
				meshMaterials[mesh.id][i] = material;
		}
		else
		{
			assert(piece < meshMaterials[mesh.id].size());
			meshMaterials[mesh.id][piece] = material;
		}
	}

#ifdef SF_DEBUG
	void APIENTRY glDebugOutput(GLenum source,
		GLenum type,
		uint32_t id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " << message << std::endl;

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;
	}
#endif
}


bool sf::Renderer::Initialize(const Window& windowArg)
{
	window = &windowArg;

	if (!gladLoadGLLoader((GLADloadproc)window->GetOpenGlFunctionAddress()))
	{
		std::cout << "[Renderer] Failed to initialize OpenGL context (GLAD)" << std::endl;
		return false;
	}

	window->AddOnResizeCallback(OnResize);

#ifdef SF_DEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	// Get GPU info and supported OpenGL version
	std::cout << "[Renderer] Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "[Renderer] OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	const glm::vec4& clearColor = Config::GetClearColor();
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glViewport(0, 0, sf::Config::GetWindowSize().x, sf::Config::GetWindowSize().y);

	sf::Renderer::aspectRatio = (float)sf::Config::GetWindowSize().x / (float)sf::Config::GetWindowSize().y;

	defaultShader.CreateFromFiles("assets/shaders/defaultV.glsl", "assets/shaders/defaultF.glsl");
	defaultMaterial.CreateFromShader(&defaultShader, false);
	defaultSkinningShader.CreateFromFiles("assets/shaders/defaultSkinningV.glsl", "assets/shaders/defaultF.glsl");
	defaultSkinningMaterial.CreateFromShader(&defaultSkinningShader, false);
	voxelBoxShader.CreateFromFiles("assets/shaders/voxelBoxV.glsl", "assets/shaders/uvF.glsl");

	glGenBuffers(1, &sharedGpuData_gl_ubo);

	rendererUniformVector.resize(3);
	rendererUniformVector[(uint32_t)RendererUniformData::BrdfLUT] = &environmentData.lookupTexture;
	rendererUniformVector[(uint32_t)RendererUniformData::PrefilterMap] = &environmentData.prefilterCubemap;
	rendererUniformVector[(uint32_t)RendererUniformData::IrradianceMap] = &environmentData.irradianceCubemap;

	// sprites
	spriteShader.CreateFromFiles("assets/shaders/spriteV.glsl", "assets/shaders/spriteF.glsl");

	// quad uvs and indices won't change
	spriteQuad.vertices[1] = { 0.0f, 0.0f };
	spriteQuad.vertices[3] = { 0.0f, 1.0f };
	spriteQuad.vertices[5] = { 1.0f, 1.0f };
	spriteQuad.vertices[7] = { 1.0f, 0.0f };

	spriteQuad.indices[0] = 0;
	spriteQuad.indices[1] = 1;
	spriteQuad.indices[2] = 2;
	spriteQuad.indices[3] = 2;
	spriteQuad.indices[4] = 3;
	spriteQuad.indices[5] = 0;

	glGenVertexArrays(1, &spriteQuad.gl_vao);
	glGenBuffers(1, &spriteQuad.gl_vertexBuffer);
	glGenBuffers(1, &spriteQuad.gl_indexBuffer);

	glBindVertexArray(spriteQuad.gl_vao);
	glBindBuffer(GL_ARRAY_BUFFER, spriteQuad.gl_vertexBuffer);

	// update vertices
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2 * 4, spriteQuad.vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteQuad.gl_indexBuffer);
	// update indices to draw
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, spriteQuad.indices, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, (void*)sizeof(glm::vec2));

	glBindVertexArray(0);
	return true;
}

void sf::Renderer::OnResize()
{
	glViewport(0, 0, window->GetWidth(), window->GetHeight());
	aspectRatio = (float)window->GetWidth() / (float)window->GetHeight();
}

void sf::Renderer::Predraw()
{
	// clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// screen space matrix
	sharedGpuData.screenSpaceMatrix = glm::ortho(0.0f, (float)sf::Config::GetWindowSize().x, (float)sf::Config::GetWindowSize().y, 0.0f);

	// camera matrices
	assert(activeCameraEntity);

	const Transform& transformComponent = activeCameraEntity.GetComponent<Transform>();
	const Camera& cameraComponent = activeCameraEntity.GetComponent<Camera>();
	if (cameraComponent.perspective)
	{
		if (aspectRatio == aspectRatio) // only if aspectRatio is not nan, it is nan when fullscreen and not visible
			cameraProjection = glm::perspective(
				cameraComponent.fieldOfView,
				aspectRatio,
				cameraComponent.nearClippingPlane,
				cameraComponent.farClippingPlane);
	}
	else
	{
		if (aspectRatio >= 1.0)
			cameraProjection = glm::ortho(
				-aspectRatio / 2.0f * cameraComponent.orthographicScale,
				aspectRatio / 2.0f * cameraComponent.orthographicScale,
				-0.5f * cameraComponent.orthographicScale,
				0.5f * cameraComponent.orthographicScale,
				cameraComponent.nearClippingPlane,
				cameraComponent.farClippingPlane);
		else
			cameraProjection = glm::ortho(
				-0.5f * cameraComponent.orthographicScale,
				0.5f * cameraComponent.orthographicScale,
				-1.0f / aspectRatio / 2.0f * cameraComponent.orthographicScale,
				1.0f / aspectRatio / 2.0f * cameraComponent.orthographicScale,
				cameraComponent.nearClippingPlane,
				cameraComponent.farClippingPlane);
	}

	cameraView = (glm::mat4)glm::conjugate(transformComponent.rotation);
	cameraView = glm::translate(cameraView, -transformComponent.position);

	sharedGpuData.cameraMatrix = cameraProjection * cameraView;
	Transform& cameraTransform = activeCameraEntity.GetComponent<Transform>();
	sharedGpuData.cameraPosition = cameraTransform.position;
}

void sf::Renderer::SetMeshMaterial(const Mesh& mesh, uint32_t materialId, int piece)
{
	assert(materialId < materials.size());
	SetMeshMaterial(mesh, materials[materialId]);
}

uint32_t sf::Renderer::CreateMaterial(const Material& material)
{
	GlMaterial* newMaterial = new GlMaterial();
	newMaterial->Create(material, rendererUniformVector);
	materials.push_back(newMaterial);
	return materials.size() - 1;
}

void sf::Renderer::SetEnvironment(const std::string& hdrFilePath, DataType hdrDataType)
{
	std::cout << "[Renderer] Loading environment: " << hdrFilePath << std::endl;
	assert(hdrDataType == DataType::f16 || hdrDataType == DataType::f32);

	if (!environmentData.lookupTexture.isInitialized)
		IblHelper::GenerateLUT(environmentData.lookupTexture, hdrDataType);
	IblHelper::CubemapFromHdr(hdrFilePath, environmentData.envCubemap, hdrDataType);
	IblHelper::SpecularFromEnv(environmentData.envCubemap, environmentData.prefilterCubemap, hdrDataType);
	IblHelper::IrradianceFromEnv(environmentData.envCubemap, environmentData.irradianceCubemap, hdrDataType);

	GlSkybox::SetCubemap(&(environmentData.envCubemap));
}

void sf::Renderer::DrawSkybox()
{
	if (drawSkybox)
		GlSkybox::Draw(cameraView, cameraProjection);
}

void sf::Renderer::DrawMesh(Mesh& mesh, Transform& transform)
{
	glEnable(GL_DEPTH_TEST);
	if (mesh.meshData->vertexCount == 0)
		return;

	assert(activeCameraEntity);

	if (meshGpuData.find(mesh.meshData) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(mesh.meshData);
	if (meshMaterials.find(mesh.id) == meshMaterials.end())
		CreateMeshMaterialSlots(mesh.id, mesh.meshData);

	sharedGpuData.modelMatrix = transform.ComputeMatrix();
	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	for (uint32_t i = 0; i < mesh.meshData->pieces.size(); i++)
	{
		GlMaterial* materialToUse = meshMaterials[mesh.id][i];

		materialToUse->Bind();

		uint32_t drawEnd, drawStart;
		drawStart = mesh.meshData->pieces[i];
		drawEnd = mesh.meshData->pieces.size() > i + 1 ? mesh.meshData->pieces[i + 1] : mesh.meshData->indexVector.size();

		glBindVertexArray(meshGpuData[mesh.meshData].gl_vao);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
		glDrawElements(GL_TRIANGLES, drawEnd - drawStart, GL_UNSIGNED_INT, (void*)(drawStart * sizeof(uint32_t)));
	}
}

void sf::Renderer::DrawSkinnedMesh(SkinnedMesh& mesh, Transform& transform)
{
	if (skeletonSsbos.find(mesh.skeletonData) == skeletonSsbos.end()) // create skeleton ssbo if not there
	{
		uint32_t newSsbo;
		glGenBuffers(1, &newSsbo);
		skeletonSsbos[mesh.skeletonData] = newSsbo;
	}

	glEnable(GL_DEPTH_TEST);
	if (mesh.meshData->vertexCount == 0)
		return;

	assert(activeCameraEntity);

	if (meshGpuData.find(mesh.meshData) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(mesh.meshData);
	if (meshMaterials.find(mesh.id) == meshMaterials.end())
		CreateSkinnedMeshMaterialSlots(mesh.id, mesh.meshData);

	sharedGpuData.modelMatrix = transform.ComputeMatrix();
	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, skeletonSsbos[mesh.skeletonData]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, mesh.skeletonData->skinningMatrices.size() * sizeof(glm::mat4), &(mesh.skeletonData->skinningMatrices[0][0][0]), GL_DYNAMIC_DRAW);

	for (uint32_t i = 0; i < mesh.meshData->pieces.size(); i++)
	{
		GlMaterial* materialToUse = meshMaterials[mesh.id][i];

		materialToUse->Bind();
		materialToUse->m_shader->SetUniform1i("animate", mesh.skeletonData->animate);

		uint32_t drawEnd, drawStart;
		drawStart = mesh.meshData->pieces[i];
		drawEnd = mesh.meshData->pieces.size() > i + 1 ? mesh.meshData->pieces[i + 1] : mesh.meshData->indexVector.size();

		glBindVertexArray(meshGpuData[mesh.meshData].gl_vao);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, skeletonSsbos[mesh.skeletonData]);
		glDrawElements(GL_TRIANGLES, drawEnd - drawStart, GL_UNSIGNED_INT, (void*)(drawStart * sizeof(uint32_t)));
	}
}

void sf::Renderer::DrawVoxelBox(VoxelBox& voxelBox, Transform& transform)
{
	glEnable(GL_DEPTH_TEST);
	assert(activeCameraEntity);

	if (meshGpuData.find(&Defaults::cubeMeshData) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(&Defaults::cubeMeshData);

	if (voxelBoxGpuData.find(voxelBox.voxelBoxData) == voxelBoxGpuData.end())
		CreateVoxelBoxGpuData(voxelBox.voxelBoxData, transform);

	// draw instanced box
	GlShader* shaderToUse = &voxelBoxShader;
	shaderToUse->Bind();

	sharedGpuData.modelMatrix = transform.ComputeMatrix();
	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	glBindVertexArray(meshGpuData[&Defaults::cubeMeshData].gl_vao);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, voxelBoxGpuData[voxelBox.voxelBoxData].gl_ssbo);
	glDrawElementsInstanced(GL_TRIANGLES, Defaults::cubeMeshData.indexVector.size(), GL_UNSIGNED_INT, (void*)0, voxelBoxGpuData[voxelBox.voxelBoxData].numberOfCubes);
}

void sf::Renderer::DrawSkeleton(Skeleton& skeleton, Transform& transform)
{
	glEnable(GL_DEPTH_TEST);

	assert(activeCameraEntity);

	if (meshGpuData.find(&Defaults::cubeMeshData) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(&Defaults::cubeMeshData);

	GlShader* shaderToUse = &defaultShader;
	shaderToUse->Bind();

	glm::mat4 worldMatrix = transform.ComputeMatrix();
	glm::mat4* boneMatrices = (glm::mat4*)alloca(sizeof(glm::mat4) * skeleton.skeletonData->bones.size());

	for (uint32_t i = 0; i < skeleton.skeletonData->bones.size(); i++)
	{
		const Bone* currentBone = &(skeleton.skeletonData->bones[i]);
		if (currentBone->parent < 0)
			boneMatrices[i] = worldMatrix * currentBone->localMatrix;
		else
			boneMatrices[i] = boneMatrices[currentBone->parent] * currentBone->localMatrix;

		sharedGpuData.modelMatrix = boneMatrices[i];
		sharedGpuData.modelMatrix = glm::scale(sharedGpuData.modelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
		glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

		glBindVertexArray(meshGpuData[&Defaults::cubeMeshData].gl_vao);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
		glDrawElements(GL_TRIANGLES, Defaults::cubeMeshData.indexVector.size(), GL_UNSIGNED_INT, (void*)0);
	}
}

void sf::Renderer::DrawSprite(Sprite& sprite, ScreenCoordinates& screenCoordinates)
{
	glDisable(GL_DEPTH_TEST);

	if (spriteTextures.find(sprite.bitmap) == spriteTextures.end()) // create mesh data if not there
		spriteTextures[sprite.bitmap].CreateFromBitmap(*sprite.bitmap, GlTexture::ClampToEdge, false);

	spriteQuad.vertices[0] = screenCoordinates.origin * glm::vec2(sf::Config::GetWindowSize().x, sf::Config::GetWindowSize().y) + (glm::vec2)screenCoordinates.offset;
	spriteQuad.vertices[2] = screenCoordinates.origin * glm::vec2(sf::Config::GetWindowSize().x, sf::Config::GetWindowSize().y) + (glm::vec2)screenCoordinates.offset + glm::vec2(0.0f, (float)(sprite.bitmap->height));
	spriteQuad.vertices[4] = screenCoordinates.origin * glm::vec2(sf::Config::GetWindowSize().x, sf::Config::GetWindowSize().y) + (glm::vec2)screenCoordinates.offset + glm::vec2((float)(sprite.bitmap->width), (float)(sprite.bitmap->height));
	spriteQuad.vertices[6] = screenCoordinates.origin * glm::vec2(sf::Config::GetWindowSize().x, sf::Config::GetWindowSize().y) + (glm::vec2)screenCoordinates.offset + glm::vec2((float)(sprite.bitmap->width), 0.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	spriteShader.Bind();
	spriteTextures[sprite.bitmap].Bind(0);
	spriteShader.SetUniform1i("bitmap", 0);

	glBindVertexArray(spriteQuad.gl_vao);
	glBindBuffer(GL_ARRAY_BUFFER, spriteQuad.gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2 * 4, spriteQuad.vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteQuad.gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, spriteQuad.indices, GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void sf::Renderer::Terminate()
{
	for (auto& pair : meshGpuData)
	{
		glDeleteVertexArrays(1, &(pair.second.gl_vao));
		glDeleteBuffers(1, &(pair.second.gl_indexBuffer));
		glDeleteBuffers(1, &(pair.second.gl_vertexBuffer));
	}

	for (GlMaterial* material : materials)
	{
		delete material;
	}
}

#endif