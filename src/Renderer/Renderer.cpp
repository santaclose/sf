#include "Renderer.h"

#include <iostream>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Config.h>

#include <glad/glad.h>
#include <Components/Camera.h>
#include <Skybox.h>
#include <Renderer/Vertex.h>
#include <Defaults.h>

namespace sf::Renderer {

	float aspectRatio = 1.7777777777;
	Entity activeCameraEntity;

	glm::mat4 cameraView;
	glm::mat4 cameraProjection;
	glm::mat4 cameraMatrix;

	struct MeshData {

		unsigned int gl_vertexBuffer;
		unsigned int gl_indexBuffer;
		unsigned int gl_vao;

		std::vector<Material*> materials;
	};

	std::unordered_map<int, MeshData> meshData;

	void TransferVertexData(int id, const sf::MeshData& mesh)
	{
		glBindBuffer(GL_ARRAY_BUFFER, meshData[id].gl_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh.vertexVector.size() * sizeof(Vertex), &mesh.vertexVector[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData[id].gl_indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexVector.size() * sizeof(unsigned int), &mesh.indexVector[0], GL_STATIC_DRAW);
	}

	void CreateMeshData(int id, const sf::MeshData& mesh)
	{
		meshData[id] = MeshData();
		for (int i = 0; i < mesh.pieces.size(); i++)
			meshData[id].materials.push_back(&Defaults::material);

		glGenVertexArrays(1, &meshData[id].gl_vao);
		glGenBuffers(1, &meshData[id].gl_vertexBuffer);
		glGenBuffers(1, &meshData[id].gl_indexBuffer);

		glBindVertexArray(meshData[id].gl_vao);
		glBindBuffer(GL_ARRAY_BUFFER, meshData[id].gl_vertexBuffer);

		// update vertices
		glBufferData(GL_ARRAY_BUFFER, mesh.vertexVector.size() * sizeof(Vertex), &mesh.vertexVector[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData[id].gl_indexBuffer);
		// update indices to draw
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indexVector.size() * sizeof(unsigned int), &mesh.indexVector[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
		glEnableVertexAttribArray(2); // tangent
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
		glEnableVertexAttribArray(3); // bitangent
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9));
		glEnableVertexAttribArray(4); // texture coords
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 12));
		glEnableVertexAttribArray(5); // extra data
		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 14));

		glBindVertexArray(0);

		TransferVertexData(id, mesh);
	}

#ifdef SF_DEBUG
	void APIENTRY glDebugOutput(GLenum source,
		GLenum type,
		unsigned int id,
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


bool sf::Renderer::Initialize(void* process)
{
	if (!gladLoadGLLoader((GLADloadproc)process))
	{
		std::cout << "[Renderer] Failed to initialize OpenGL context (GLAD)" << std::endl;
		return false;
	}

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

	glClearColor(sf::Config::clearColor[0], sf::Config::clearColor[1], sf::Config::clearColor[2], 0.0);
	glViewport(0, 0, sf::Config::windowWidth, sf::Config::windowHeight);

	sf::Renderer::aspectRatio = (float)sf::Config::windowWidth / (float)sf::Config::windowHeight;
	return true;
}

void sf::Renderer::OnResize()
{
	glViewport(0, 0, Config::windowWidth, Config::windowHeight);
	aspectRatio = (float)Config::windowWidth / (float)Config::windowHeight;
}

void sf::Renderer::ClearBuffers()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void sf::Renderer::ComputeCameraMatrices()
{
	assert(activeCameraEntity);

	const Transform& transformComponent = activeCameraEntity.GetComponent<Transform>();
	const Camera& cameraComponent = activeCameraEntity.GetComponent<Camera>();
	if (cameraComponent.perspective)
	{
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

	cameraMatrix = cameraProjection * cameraView;
}

void sf::Renderer::SetMeshMaterial(Mesh mesh, Material* material, int piece)
{
	if (meshData.find(mesh.id) == meshData.end()) // create mesh data if not there
		CreateMeshData(mesh.id, *(mesh.meshData));

	assert(piece < meshData[mesh.id].materials.size());
	meshData[mesh.id].materials[piece] = material;
}

void sf::Renderer::DrawSkybox()
{
	Skybox::Draw(cameraView, cameraProjection);
}

void sf::Renderer::DrawMesh(Mesh& mesh, Transform& transform)
{
	if (mesh.meshData->vertexVector.size() == 0)
		return;

	assert(activeCameraEntity);

	if (meshData.find(mesh.id) == meshData.end()) // create mesh data if not there
		CreateMeshData(mesh.id, *(mesh.meshData));

	Transform& cameraTransform = activeCameraEntity.GetComponent<Transform>();
	for (unsigned int i = 0; i < mesh.meshData->pieces.size(); i++)
	{
		Material* materialToUse = meshData[mesh.id].materials[i];

		materialToUse->Bind();

		materialToUse->m_shader->SetUniformMatrix4fv("cameraMatrix", &(cameraMatrix[0][0]));
		materialToUse->m_shader->SetUniform3fv("camPos", &(cameraTransform.position.x));
		materialToUse->m_shader->SetUniformMatrix4fv("modelMatrix", &(transform.ComputeMatrix()[0][0]));

		unsigned int drawEnd, drawStart;
		drawStart = mesh.meshData->pieces[i];
		drawEnd = mesh.meshData->pieces.size() > i + 1 ? mesh.meshData->pieces[i + 1] : mesh.meshData->indexVector.size();

		glBindVertexArray(meshData[mesh.id].gl_vao);
		glDrawElements(GL_TRIANGLES, drawEnd - drawStart, GL_UNSIGNED_INT, (void*)(drawStart * sizeof(unsigned int)));
	}
}

void sf::Renderer::DrawVoxelBox(VoxelBox& voxelBox, Transform& transform)
{
	const VoxelBoxData* voxelBoxData = voxelBox.voxelBoxData;
	assert(activeCameraEntity);

	if (meshData.find(-1) == meshData.end()) // create mesh data if not there
		CreateMeshData(-1, Defaults::cubeMeshData);

	// draw instanced box
	Transform& cameraTransform = activeCameraEntity.GetComponent<Transform>();

	Material* materialToUse = meshData[-1].materials[0];
	materialToUse->Bind();

	materialToUse->m_shader->SetUniformMatrix4fv("cameraMatrix", &(cameraMatrix[0][0]));
	materialToUse->m_shader->SetUniform3fv("camPos", &(cameraTransform.position.x));

	Transform voxelSpaceCursor;
	voxelSpaceCursor.scale = voxelBox.voxelBoxData->voxelSize;

	for (int i = 0; i < voxelBox.voxelBoxData->mat.size(); i++)
	{
		for (int j = 0; j < voxelBox.voxelBoxData->mat[0].size(); j++)
		{
			for (int k = 0; k < voxelBox.voxelBoxData->mat[0][0].size(); k++)
			{
				if (voxelBox.voxelBoxData->mat[i][j][k])
				{
					voxelSpaceCursor.position = voxelBox.voxelBoxData->offset;
					voxelSpaceCursor.position.x += voxelBox.voxelBoxData->voxelSize * ((float)i + 0.5f);
					voxelSpaceCursor.position.y += voxelBox.voxelBoxData->voxelSize * ((float)j + 0.5f);
					voxelSpaceCursor.position.z += voxelBox.voxelBoxData->voxelSize * ((float)k + 0.5f);

					glm::mat4 shaderMatrix = transform.ComputeMatrix() * voxelSpaceCursor.ComputeMatrix();

					materialToUse->m_shader->SetUniformMatrix4fv("modelMatrix", &(shaderMatrix[0][0]));
					glBindVertexArray(meshData[-1].gl_vao);
					glDrawElements(GL_TRIANGLES, Defaults::cubeMeshData.indexVector.size(), GL_UNSIGNED_INT, (void*)0);
				}
			}
		}
	}
}

void sf::Renderer::Terminate()
{
	for (auto& pair : meshData)
	{
		glDeleteVertexArrays(1, &(pair.second.gl_vao));
		glDeleteBuffers(1, &(pair.second.gl_indexBuffer));
		glDeleteBuffers(1, &(pair.second.gl_vertexBuffer));
	}
}
