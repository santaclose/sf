#ifdef SF_USE_OPENGL

#include "../ImGuiBind.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

void sf::Renderer::ImGuiBind::Initialize(GLFWwindow* window)
{
}
void sf::Renderer::ImGuiBind::AfterConfigure(GLFWwindow* window)
{
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
}


void  sf::Renderer::ImGuiBind::NewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
}

void  sf::Renderer::ImGuiBind::FrameRender(ImDrawData* draw_data)
{
	ImGui_ImplOpenGL3_RenderDrawData(draw_data);
}

void  sf::Renderer::ImGuiBind::OnResize() {}

void  sf::Renderer::ImGuiBind::Terminate() {}

#endif