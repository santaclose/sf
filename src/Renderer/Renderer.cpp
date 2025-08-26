#include "Renderer.h"

#include <assert.h>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>
#include <Components/Camera.h>

#include <Material.h>
#include <Defaults.h>
#include <Bitmap.h>
#include <Hash.h>

#include <Renderer/GlSkybox.h>
#include <Renderer/IblHelper.h>

#include <SebTextFontData.h>
#include <SebTextTextData.h>
#include <SebTextRenderData.h>

namespace sf::Renderer
{
	const Window* window;

	BufferLayout positionColorVertexLayout = BufferLayout({
		BufferComponent::VertexPosition,
		BufferComponent::VertexColor
	});
	BufferLayout positionUvVertexLayout = BufferLayout({
		BufferComponent::VertexPosition,
		BufferComponent::VertexUV
	});

	GlShader drawLineShader;

	float aspectRatio;
	Entity activeCameraEntity;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;

	glm::vec3 clearColor;

	struct MeshGpuData
	{
		uint32_t gl_vertexBuffer;
		uint32_t gl_indexBuffer;
		uint32_t gl_vao;
	};

	struct ParticleGpuData
	{
		uint32_t gl_ssbo;
		std::vector<uint8_t> perParticleData;
	};

	struct ParticleSystemData
	{
		float emissionTimer = 0.0f;
		float cycleCurrentTime = 0.0f;
		uint32_t currentParticle = 0;
		uint32_t activeParticles = 0;
		ParticleGpuData gpuData;
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
	GlShader spriteShader;

	struct TextGpuData
	{
		SebText::TextData textData;
		uint32_t gl_ssbo_perInstanceData;
		uint32_t gl_ssbo_bezierData;
		uint32_t gl_ssbo_glyphMetaData;
		uint32_t gl_ssbo_lastCharPerLine;
		uint32_t gl_ubo_layoutData;
		TextGpuData(const std::string& text, const SebText::FontData& fontData) : textData(text, fontData) {}
	};
	MeshGpuData textMeshGpuData = { ~0U, ~0U, ~0U };
	std::unordered_map<unsigned, SebText::FontData> fontPathToFontData;
	std::unordered_map<unsigned, std::unordered_map<unsigned, TextGpuData>> fontPathAndStringToTextData;
	std::vector<SebText::GlyphRenderData> prevGlyphRenderData;
	SebText::LayoutSettings textLayoutSettings;
	GlShader textShader;

	struct SharedGpuData
	{
		glm::mat4 modelMatrix;
		glm::mat4 cameraMatrix;
		glm::vec3 cameraPosition;
		glm::vec2 windowSize;
	};
	uint32_t sharedGpuData_gl_ubo;
	SharedGpuData sharedGpuData;

	std::unordered_map<const sf::SkeletonData*, uint32_t> skeletonSsbos;

	std::unordered_map<const sf::MeshData*, MeshGpuData> meshGpuData;

	std::unordered_map<void*, ParticleSystemData> particleSystemData;

	std::unordered_map<const sf::VoxelVolumeData*, uint32_t> voxelVolumeSsbos;

	std::unordered_map<uint32_t, const BufferLayout*> particleBufferLayouts;

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

	struct LineVertex {
		glm::vec3 pos;
		glm::vec3 color;
	};
	std::vector<LineVertex> drawLineLines;
	bool drawLineDataInitialized = false;
	uint32_t drawLineVBO, drawLineVAO;

	bool debugDrawEnabled = false;
	glm::vec3 debugDrawColor = { 0.0f, 0.0f, 0.0f };

	void TransferVertexData(const sf::MeshData* mesh)
	{
		glBindBuffer(GL_ARRAY_BUFFER, meshGpuData[mesh].gl_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * mesh->vertexBufferLayout.GetSize(), mesh->vertexBuffer, GL_STATIC_DRAW);

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
		glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * mesh->vertexBufferLayout.GetSize(), mesh->vertexBuffer, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshGpuData[mesh].gl_indexBuffer);
		// update indices to draw
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexVector.size() * sizeof(uint32_t), &mesh->indexVector[0], GL_STATIC_DRAW);

		const std::vector<BufferComponentInfo>& components = mesh->vertexBufferLayout.GetComponentInfos();
		for (int i = 0; i < components.size(); i++)
		{
			glEnableVertexAttribArray(i);
			switch (components[i].dataType)
			{
				case DataType::f32:
					glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, mesh->vertexBufferLayout.GetSize(), (void*)(uint64_t)components[i].byteOffset);
					break;
				case DataType::vec2f32:
					glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, mesh->vertexBufferLayout.GetSize(), (void*)(uint64_t)components[i].byteOffset);
					break;
				case DataType::vec3f32:
					glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, mesh->vertexBufferLayout.GetSize(), (void*)(uint64_t)components[i].byteOffset);
					break;
				case DataType::vec4f32:
					glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, mesh->vertexBufferLayout.GetSize(), (void*)(uint64_t)components[i].byteOffset);
					break;
				case DataType::vec4u8:
					glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_FALSE, mesh->vertexBufferLayout.GetSize(), (void*)(uint64_t)components[i].byteOffset);
					break;
				case DataType::vec4u16:
					glVertexAttribPointer(i, 4, GL_UNSIGNED_SHORT, GL_FALSE, mesh->vertexBufferLayout.GetSize(), (void*)(uint64_t)components[i].byteOffset);
					break;
				default:
					std::cout << "[Renderer] Vertex attribute skipped" << std::endl;
					assert(false);
					break;
			}
		}

		TransferVertexData(mesh);
		glBindVertexArray(0);
	}

	void CreateSpriteGpuData()
	{
		spriteShader.CreateFromFiles("assets/shaders/sprite.vert", "assets/shaders/sprite.frag", positionUvVertexLayout);

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
	}

	void CreateTextGpuData(const char* fontPath, const char* string, const SebText::TextRenderData& trd, const SebText::TextData& td)
	{
		unsigned fontPathHash = Hash::SimpleStringHash(fontPath);
		unsigned stringHash = Hash::SimpleStringHash(string);

		std::vector<SebText::InstanceData> instanceData;
		SebText::CreateInstanceData(instanceData, fontPathAndStringToTextData[fontPathHash].at(stringHash).textData, prevGlyphRenderData, textLayoutSettings);

		glGenBuffers(1, &(fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_perInstanceData));
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_perInstanceData);
		glBufferData(GL_SHADER_STORAGE_BUFFER, instanceData.size() * sizeof(SebText::InstanceData), instanceData.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &(fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_bezierData));
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_bezierData);
		glBufferData(GL_SHADER_STORAGE_BUFFER, trd.BezierPoints.size() * sizeof(glm::vec2), trd.BezierPoints.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &(fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_glyphMetaData));
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_glyphMetaData);
		glBufferData(GL_SHADER_STORAGE_BUFFER, trd.GlyphMetaData.size() * sizeof(int), trd.GlyphMetaData.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &(fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_lastCharPerLine));
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_lastCharPerLine);
		glBufferData(GL_SHADER_STORAGE_BUFFER, td.LastCharacterPerLine.size() * sizeof(int), td.LastCharacterPerLine.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &(fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ubo_layoutData));
		glBindBuffer(GL_UNIFORM_BUFFER, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ubo_layoutData);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SebText::LayoutSettings), &textLayoutSettings, GL_DYNAMIC_DRAW);
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


bool sf::Renderer::Initialize(const Window& windowArg, const glm::vec3& clearColorArg)
{
	clearColor = clearColorArg;
	window = &windowArg;

	materials.clear();
	materials.reserve(64);

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

	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	glPolygonMode(GL_FRONT, GL_FILL);
	glCullFace(GL_BACK);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glClearColor(clearColor.r, clearColor.g, clearColor.b, 0.0f);
	glViewport(0, 0, window->GetWidth(), window->GetHeight());

	sf::Renderer::aspectRatio = (float)(window->GetWidth()) / (float)(window->GetHeight());

	glGenBuffers(1, &sharedGpuData_gl_ubo);

	rendererUniformVector.resize(3);
	rendererUniformVector[(uint32_t)RendererUniformData::BrdfLUT] = &environmentData.lookupTexture;
	rendererUniformVector[(uint32_t)RendererUniformData::PrefilterMap] = &environmentData.prefilterCubemap;
	rendererUniformVector[(uint32_t)RendererUniformData::IrradianceMap] = &environmentData.irradianceCubemap;

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

	sharedGpuData.windowSize = glm::vec2((float)window->GetWidth(), (float)window->GetHeight());

	if (!activeCameraEntity)
		return;

	// camera matrices
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
	sharedGpuData.cameraPosition = transformComponent.position;
}

void sf::Renderer::Postdraw()
{
	drawLineLines.clear();
}

void sf::Renderer::SetClearColor(const glm::vec3& clearColorArg)
{
	clearColor = clearColorArg;
	glClearColor(clearColor.r, clearColor.g, clearColor.b, 0.0f);
}

const glm::vec3& sf::Renderer::GetClearColor()
{
	return clearColor;
}

void sf::Renderer::SetActiveCameraEntity(Entity cameraEntity)
{
	activeCameraEntity = cameraEntity;
}

sf::Entity sf::Renderer::GetActiveCameraEntity()
{
	return activeCameraEntity;
}

uint32_t sf::Renderer::CreateMaterial(const Material& material, const BufferLayout& vertexBufferLayout, const BufferLayout* voxelBufferLayout, const BufferLayout* particleBufferLayout)
{
	GlMaterial* newMaterial = new GlMaterial();
	newMaterial->Create(material, rendererUniformVector, vertexBufferLayout, voxelBufferLayout, particleBufferLayout);
	materials.push_back(newMaterial);
	uint32_t newId = materials.size() - 1;
	particleBufferLayouts[newId] = particleBufferLayout;
	return newId;
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
	GlSkybox::Draw(cameraView, cameraProjection);
}

void sf::Renderer::DrawMesh(Mesh& mesh, Transform& transform)
{
	if (mesh.meshData->vertexCount == 0)
		return;

	if (!activeCameraEntity)
		return;

	if (meshGpuData.find(mesh.meshData) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(mesh.meshData);

	sharedGpuData.modelMatrix = transform.ComputeMatrix();
	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	for (uint32_t i = 0; i < mesh.meshData->pieces.size(); i++)
	{
		GlMaterial* materialToUse = materials[mesh.materials[i]];

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
	assert(mesh.skeletonData != nullptr);

	if (skeletonSsbos.find(mesh.skeletonData) == skeletonSsbos.end()) // create skeleton ssbo if not there
	{
		uint32_t newSsbo;
		glGenBuffers(1, &newSsbo);
		skeletonSsbos[mesh.skeletonData] = newSsbo;
	}

	if (mesh.meshData->vertexCount == 0)
		return;

	if (!activeCameraEntity)
		return;

	if (meshGpuData.find(mesh.meshData) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(mesh.meshData);

	sharedGpuData.modelMatrix = transform.ComputeMatrix();
	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, skeletonSsbos[mesh.skeletonData]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, mesh.skeletonData->m_skinningMatrices.size() * sizeof(glm::mat4), &(mesh.skeletonData->m_skinningMatrices[0][0][0]), GL_DYNAMIC_DRAW);

	for (uint32_t i = 0; i < mesh.meshData->pieces.size(); i++)
	{
		GlMaterial* materialToUse = materials[mesh.materials[i]];

		materialToUse->Bind();
		materialToUse->m_shader->SetUniform1i("animate", mesh.skeletonData->m_animate);

		uint32_t drawEnd, drawStart;
		drawStart = mesh.meshData->pieces[i];
		drawEnd = mesh.meshData->pieces.size() > i + 1 ? mesh.meshData->pieces[i + 1] : mesh.meshData->indexVector.size();

		glBindVertexArray(meshGpuData[mesh.meshData].gl_vao);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, skeletonSsbos[mesh.skeletonData]);
		glDrawElements(GL_TRIANGLES, drawEnd - drawStart, GL_UNSIGNED_INT, (void*)(drawStart * sizeof(uint32_t)));
	}

	if (debugDrawEnabled)
		DebugDrawSkeleton(mesh, transform);
}

void sf::Renderer::DrawParticleSystem(ParticleSystem& particleSystem, Transform& transform, float deltaTime)
{
	if (!activeCameraEntity)
		return;

	GlMaterial* materialToUse = materials[particleSystem.material];
	const BufferLayout* particleBufferLayout = particleBufferLayouts[particleSystem.material];

	glDepthMask(GL_FALSE); // Do not write to depth buffer

	if (meshGpuData.find(particleSystem.meshData) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(particleSystem.meshData);

	float cycleTotalTime = particleSystem.timeBetweenEmissions * (particleSystem.particleCount / particleSystem.particlesPerEmission);
	if (particleSystemData.find(&particleSystem) == particleSystemData.end())
	{
		ParticleGpuData& particleGpuData = particleSystemData[&particleSystem].gpuData;
		assert((particleSystem.particleCount % particleSystem.particlesPerEmission) == 0);
		particleGpuData = ParticleGpuData();
		uint32_t gpuDataSizeInBytes = particleSystem.particleCount * particleBufferLayout->GetSize();
		particleGpuData.perParticleData.resize(gpuDataSizeInBytes);
		memset(particleGpuData.perParticleData.data(), 0, gpuDataSizeInBytes);
		cycleTotalTime = particleSystem.timeBetweenEmissions * particleSystem.particleCount;
		glGenBuffers(1, &(particleGpuData.gl_ssbo));
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleGpuData.gl_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, gpuDataSizeInBytes, particleGpuData.perParticleData.data(), GL_DYNAMIC_DRAW);
	}

	ParticleGpuData& particleGpuData = particleSystemData[&particleSystem].gpuData;
	void* perParticleBuffer = particleGpuData.perParticleData.data();

	if (particleSystemData[&particleSystem].emissionTimer < 0.0f)
	{
		if (particleSystem.emit)
		{
			for (uint32_t i = 0; i < particleSystem.particlesPerEmission; i++)
			{
				uint32_t emittingParticle = (particleSystemData[&particleSystem].currentParticle + i) % particleSystem.particleCount;
				glm::vec3* emittingParticlePosition = particleBufferLayout->Access<glm::vec3>(perParticleBuffer, BufferComponent::ParticlePosition, emittingParticle);
				glm::quat* emittingParticleRotation = particleBufferLayout->Access<glm::quat>(perParticleBuffer, BufferComponent::ParticleRotation, emittingParticle);
				float* emittingParticleScale = particleBufferLayout->Access<float>(perParticleBuffer, BufferComponent::ParticleScale, emittingParticle);
				float* emittingParticleSpawnTime = particleBufferLayout->Access<float>(perParticleBuffer, BufferComponent::ParticleSpawnTime, emittingParticle);

				*emittingParticlePosition = transform.position;
				*emittingParticleRotation = transform.rotation;
				*emittingParticleScale = transform.scale;
				*emittingParticleSpawnTime = particleSystemData[&particleSystem].cycleCurrentTime;

				if (particleSystem.initialTransform != nullptr)
				{
					Transform initial = particleSystem.initialTransform();
					*emittingParticlePosition += transform.rotation * initial.position;
					*emittingParticleRotation *= initial.rotation;
					*emittingParticleScale *= initial.scale;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < particleSystem.particlesPerEmission; i++)
			{
				uint32_t disablingParticle = (particleSystemData[&particleSystem].currentParticle + i) % particleSystem.particleCount;
				float* disablingParticleScale = particleBufferLayout->Access<float>(perParticleBuffer, BufferComponent::ParticleScale, disablingParticle);
				float* disablingParticleSpawnTime = particleBufferLayout->Access<float>(perParticleBuffer, BufferComponent::ParticleSpawnTime, disablingParticle);

				*disablingParticleScale = 0.0f;
				*disablingParticleSpawnTime = -1.0f;
			}
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleGpuData.gl_ssbo);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER,
			particleBufferLayout->GetSize() * particleSystemData[&particleSystem].currentParticle,
			particleBufferLayout->GetSize() * particleSystem.particlesPerEmission,
			(uint8_t*)perParticleBuffer + particleBufferLayout->GetSize() * particleSystemData[&particleSystem].currentParticle);

		particleSystemData[&particleSystem].emissionTimer += particleSystem.timeBetweenEmissions;
		if (particleSystem.emit)
			particleSystemData[&particleSystem].activeParticles = glm::max(particleSystemData[&particleSystem].activeParticles + particleSystem.particlesPerEmission, particleSystem.particleCount);
		else
			particleSystemData[&particleSystem].activeParticles = particleSystemData[&particleSystem].activeParticles == 0U ? 0U : particleSystemData[&particleSystem].activeParticles - particleSystem.particlesPerEmission;
		particleSystemData[&particleSystem].currentParticle = (particleSystemData[&particleSystem].currentParticle + particleSystem.particlesPerEmission) % particleSystem.particleCount;
	}

	if (particleSystemData[&particleSystem].activeParticles != 0)
	{
		materialToUse->Bind();

		materialToUse->m_shader->SetUniform1f("PARTICLE_CYCLE_TIME", particleSystemData[&particleSystem].cycleCurrentTime);
		materialToUse->m_shader->SetUniform1f("PARTICLE_LIFETIME", cycleTotalTime);

		sharedGpuData.modelMatrix = transform.ComputeMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

		glBindVertexArray(meshGpuData[particleSystem.meshData].gl_vao);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleGpuData.gl_ssbo);
		glDrawElementsInstanced(GL_TRIANGLES, particleSystem.meshData->indexVector.size(), GL_UNSIGNED_INT, (void*)0, particleSystem.particleCount);
	}

	particleSystemData[&particleSystem].emissionTimer -= deltaTime;
	particleSystemData[&particleSystem].cycleCurrentTime = glm::mod(particleSystemData[&particleSystem].cycleCurrentTime + deltaTime, cycleTotalTime);

	glDepthMask(GL_TRUE); // Restore depth mask
}

void sf::Renderer::DrawVoxelVolume(VoxelVolume& voxelVolume, Transform& transform)
{
	if (!activeCameraEntity)
		return;

	if (meshGpuData.find(&Defaults::MeshDataCube()) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(&Defaults::MeshDataCube());

	if (voxelVolumeSsbos.find(voxelVolume.voxelVolumeData) == voxelVolumeSsbos.end())
	{
		uint32_t newSsbo;
		glGenBuffers(1, &newSsbo);
		voxelVolumeSsbos[voxelVolume.voxelVolumeData] = newSsbo;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxelVolumeSsbos[voxelVolume.voxelVolumeData]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, voxelVolume.voxelVolumeData->voxelBuffer.size(), voxelVolume.voxelVolumeData->voxelBuffer.data(), GL_STATIC_DRAW);
	}

	GlMaterial* materialToUse = materials[voxelVolume.material];

	materialToUse->Bind();
	materialToUse->m_shader->SetUniform1f("voxelSize", voxelVolume.voxelVolumeData->voxelSize);

	// draw instanced box
	sharedGpuData.modelMatrix = transform.ComputeMatrix();
	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	glBindVertexArray(meshGpuData[&Defaults::MeshDataCube()].gl_vao);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, voxelVolumeSsbos[voxelVolume.voxelVolumeData]);
	glDrawElementsInstanced(GL_TRIANGLES, Defaults::MeshDataCube().indexVector.size(), GL_UNSIGNED_INT, (void*)0, voxelVolume.voxelVolumeData->GetVoxelCount());
}

void sf::Renderer::DrawSprite(Sprite& sprite, ScreenCoordinates& screenCoordinates)
{
	glDisable(GL_DEPTH_TEST);

	if (!spriteShader.Initialized())
		CreateSpriteGpuData();

	if (spriteTextures.find(sprite.bitmap) == spriteTextures.end()) // create mesh data if not there
		spriteTextures[sprite.bitmap].CreateFromBitmap(*sprite.bitmap, GlTexture::ClampToEdge, false);

	glm::vec2 spriteTopLeft = screenCoordinates.origin * glm::vec2(window->GetWidth(), window->GetHeight()) + (glm::vec2)screenCoordinates.offset;
	if (sprite.alignmentH != ALIGNMENT_LEFT) spriteTopLeft.x -= ((float)sprite.bitmap->width) * (sprite.alignmentH * 0.5f);
	if (sprite.alignmentV != ALIGNMENT_LEFT) spriteTopLeft.y -= ((float)sprite.bitmap->height) * (sprite.alignmentV * 0.5f);
	spriteTopLeft = glm::vec2(glm::round(spriteTopLeft.x), glm::round(spriteTopLeft.y));
	spriteQuad.vertices[2] = spriteTopLeft;
	spriteQuad.vertices[0] = spriteTopLeft + glm::vec2(0.0f, (float)(sprite.bitmap->height));
	spriteQuad.vertices[6] = spriteTopLeft + glm::vec2((float)(sprite.bitmap->width), (float)(sprite.bitmap->height));
	spriteQuad.vertices[4] = spriteTopLeft + glm::vec2((float)(sprite.bitmap->width), 0.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	spriteShader.Bind();
	spriteTextures[sprite.bitmap].Bind(0);
	spriteShader.SetUniform1i("bitmap", 0);
	glPolygonMode(GL_FRONT, GL_FILL);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	glBindVertexArray(spriteQuad.gl_vao);
	glBindBuffer(GL_ARRAY_BUFFER, spriteQuad.gl_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2 * 4, spriteQuad.vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteQuad.gl_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 6, spriteQuad.indices, GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glEnable(GL_DEPTH_TEST);
}

void sf::Renderer::DrawText(Text& text, ScreenCoordinates& screenCoordinates)
{
	glDisable(GL_DEPTH_TEST);

	unsigned fontPathHash = Hash::SimpleStringHash(text.fontPath);
	unsigned stringHash = Hash::SimpleStringHash(text.string);
	if (fontPathToFontData.find(fontPathHash) == fontPathToFontData.end())
		fontPathToFontData.insert({ fontPathHash, SebText::FontData(text.fontPath) });
	if (fontPathAndStringToTextData.find(fontPathHash) == fontPathAndStringToTextData.end() || fontPathAndStringToTextData[fontPathHash].find(stringHash) == fontPathAndStringToTextData[fontPathHash].end())
	{
		fontPathAndStringToTextData.insert({ fontPathHash, std::unordered_map<unsigned, TextGpuData>() });
		fontPathAndStringToTextData[fontPathHash].insert({ stringHash, TextGpuData(text.string, fontPathToFontData.at(fontPathHash)) });
		SebText::TextData& td = fontPathAndStringToTextData[fontPathHash].at(stringHash).textData;
		if (td.PrintableCharacters.size() > 0)
		{
			SebText::TextRenderData trd = SebText::CreateRenderData(td.UniquePrintableCharacters, fontPathToFontData.at(fontPathHash));
			prevGlyphRenderData = trd.AllGlyphData;
			CreateTextGpuData(text.fontPath, text.string, trd, td);
		}
	}
	if (textMeshGpuData.gl_indexBuffer == ~0U)
	{
		glGenVertexArrays(1, &textMeshGpuData.gl_vao);
		glGenBuffers(1, &textMeshGpuData.gl_vertexBuffer);
		glGenBuffers(1, &textMeshGpuData.gl_indexBuffer);

		glBindVertexArray(textMeshGpuData.gl_vao);
		glBindBuffer(GL_ARRAY_BUFFER, textMeshGpuData.gl_vertexBuffer);

		// update vertices
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SebText::Vertex), SebText::MeshVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textMeshGpuData.gl_indexBuffer);
		// update indices to draw
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), SebText::MeshIndices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SebText::Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SebText::Vertex), (void*)(sizeof(glm::vec3)));

		glBindBuffer(GL_ARRAY_BUFFER, textMeshGpuData.gl_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SebText::Vertex), SebText::MeshVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textMeshGpuData.gl_indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), SebText::MeshIndices, GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	textLayoutSettings.FontSize = text.size;
	textLayoutSettings.AlignmentH = text.alignmentH;
	textLayoutSettings.AlignmentV = text.alignmentV;
	glBindBuffer(GL_UNIFORM_BUFFER, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ubo_layoutData);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SebText::LayoutSettings), &textLayoutSettings, GL_DYNAMIC_DRAW);

	if (!textShader.Initialized())
		textShader.CreateFromFiles("vendor/sebtext/shader.vert", "vendor/sebtext/shader.frag", positionUvVertexLayout);

	textShader.Bind();
	glPolygonMode(GL_FRONT, GL_FILL);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	textShader.SetUniform4fv("textCol", &text.color.r);
	glm::vec2 targetOffset = screenCoordinates.origin * glm::vec2(window->GetWidth(), window->GetHeight()) + (glm::vec2)screenCoordinates.offset;
	textShader.SetUniform2fv("globalOffset", &targetOffset.x);
	textShader.SetUniform1i("lineCount", fontPathAndStringToTextData[fontPathHash].at(stringHash).textData.LineCount);

	glBindVertexArray(textMeshGpuData.gl_vao);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_perInstanceData);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_bezierData);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_glyphMetaData);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ssbo_lastCharPerLine);
	glBindBufferBase(GL_UNIFORM_BUFFER, 5, fontPathAndStringToTextData[fontPathHash].at(stringHash).gl_ubo_layoutData);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0, fontPathAndStringToTextData[fontPathHash].at(stringHash).textData.PrintableCharacters.size());

	glEnable(GL_DEPTH_TEST);
}

void sf::Renderer::AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color)
{
	drawLineLines.resize(drawLineLines.size() + 2);
	drawLineLines[drawLineLines.size() - 2] = { a, color };
	drawLineLines[drawLineLines.size() - 1] = { b, color };
}

void sf::Renderer::SetDebugDrawEnabled(bool value)
{
	debugDrawEnabled = value;
}

bool sf::Renderer::IsDebugDrawEnabled()
{
	return debugDrawEnabled;
}

void sf::Renderer::DebugDrawSkeleton(SkinnedMesh& mesh, Transform& transform)
{
	if (!debugDrawEnabled)
		return;
	if (!activeCameraEntity)
		return;

	if (meshGpuData.find(&Defaults::MeshDataCube()) == meshGpuData.end()) // create mesh data if not there
		CreateMeshGpuData(&Defaults::MeshDataCube());

	glm::mat4 worldMatrix = transform.ComputeMatrix();
	glm::mat4* boneMatrices = (glm::mat4*)alloca(sizeof(glm::mat4) * mesh.skeletonData->m_bones.size());

	for (uint32_t i = 0; i < mesh.skeletonData->m_bones.size(); i++)
	{
		const Bone* currentBone = &(mesh.skeletonData->m_bones[i]);
		if (currentBone->parent < 0)
			boneMatrices[i] = worldMatrix * currentBone->localMatrix;
		else
		{
			boneMatrices[i] = boneMatrices[currentBone->parent] * currentBone->localMatrix;
			AddLine(
				boneMatrices[currentBone->parent] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
				boneMatrices[i] * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}
}

void sf::Renderer::DrawSphereCollider(const SphereCollider& sc, const glm::vec3& color)
{
	AddLine(sc.center + glm::vec3(sc.radius, 0.0f, 0.0f), sc.center - glm::vec3(sc.radius, 0.0f, 0.0f), color);
	AddLine(sc.center + glm::vec3(0.0f, sc.radius, 0.0f), sc.center - glm::vec3(0.0f, sc.radius, 0.0f), color);
	AddLine(sc.center + glm::vec3(0.0f, 0.0f, sc.radius), sc.center - glm::vec3(0.0f, 0.0f, sc.radius), color);
}

void sf::Renderer::DrawCapsuleCollider(const CapsuleCollider& cc, const glm::vec3& color)
{
	AddLine(cc.centerA + glm::vec3(cc.radius, 0.0f, 0.0f), cc.centerA - glm::vec3(cc.radius, 0.0f, 0.0f), color);
	AddLine(cc.centerA + glm::vec3(0.0f, cc.radius, 0.0f), cc.centerA - glm::vec3(0.0f, cc.radius, 0.0f), color);
	AddLine(cc.centerA + glm::vec3(0.0f, 0.0f, cc.radius), cc.centerA - glm::vec3(0.0f, 0.0f, cc.radius), color);
	AddLine(cc.centerB + glm::vec3(cc.radius, 0.0f, 0.0f), cc.centerB - glm::vec3(cc.radius, 0.0f, 0.0f), color);
	AddLine(cc.centerB + glm::vec3(0.0f, cc.radius, 0.0f), cc.centerB - glm::vec3(0.0f, cc.radius, 0.0f), color);
	AddLine(cc.centerB + glm::vec3(0.0f, 0.0f, cc.radius), cc.centerB - glm::vec3(0.0f, 0.0f, cc.radius), color);
	AddLine(cc.centerA, cc.centerB, color);
}

void sf::Renderer::DrawBoxCollider(const BoxCollider& bc, const glm::vec3& color)
{
	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), color);

	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), color);

	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(-bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, +bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, +bc.size.z * 0.5f), color);
	AddLine(bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, -bc.size.y * 0.5f, -bc.size.z * 0.5f), bc.center + bc.orientation * glm::vec3(+bc.size.x * 0.5f, +bc.size.y * 0.5f, -bc.size.z * 0.5f), color);
}

void sf::Renderer::DrawLines()
{
	if (drawLineLines.size() == 0)
		return;
	glDisable(GL_DEPTH_TEST);
	if (!drawLineDataInitialized)
	{
		glGenVertexArrays(1, &drawLineVAO);
		glGenBuffers(1, &drawLineVBO);

		glBindVertexArray(drawLineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, drawLineVBO);
		glBufferData(GL_ARRAY_BUFFER, drawLineLines.size() * sizeof(LineVertex), drawLineLines.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));

		drawLineShader.CreateFromFiles("assets/shaders/drawLines.vert", "assets/shaders/drawLines.frag", positionColorVertexLayout);

		drawLineDataInitialized = true;
	}

	drawLineShader.Bind();
	glPolygonMode(GL_FRONT, GL_FILL);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	glBindBuffer(GL_UNIFORM_BUFFER, sharedGpuData_gl_ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SharedGpuData), &sharedGpuData, GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, sharedGpuData_gl_ubo);

	glBindVertexArray(drawLineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, drawLineVBO);

	glBufferData(GL_ARRAY_BUFFER, drawLineLines.size() * sizeof(LineVertex), drawLineLines.data(), GL_DYNAMIC_DRAW);

	glDrawArrays(GL_LINES, 0, drawLineLines.size());

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
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
	drawLineShader.Delete();

	environmentData.envTexture.Delete();
	environmentData.envCubemap.Delete();
	environmentData.irradianceCubemap.Delete();
	environmentData.prefilterCubemap.Delete();
	environmentData.lookupTexture.Delete();
}
