#include <GLFW/glfw3.h>
#include <imgui.h>
#include <iostream>
#include <fstream>

#include <Game.h>
#include <Math.hpp>
#include <Random.h>
#include <Input.h>
#include <MeshProcessor.h>
#include <GameInitializationData.h>
#include <FileUtils.h>

#include <Renderer/Renderer.h>

#include <Scene/Entity.h>
#include <Scene/Scene.h>

#include <Components/ScreenCoordinates.h>
#include <Components/Text.h>


namespace sf
{
	std::string Game::ConfigFilePath = "examples/ui/config.json";

	Scene scene;
	Entity textA, textB, textC, spriteTest;

	char textStringA[256] = "Left aligned\0";
	char textStringB[256] = "Right aligned\0";
	char textStringC[256] = "Centered multiline\ntext\0";

	const char* testSpriteBitmapPath = "C:/Users/san/Desktop/test.png";
	Bitmap* testSpriteBitmap;

	void DownloadAssetDependencies(const std::vector<std::string>& urls, const std::string& targetPath)
	{
		for (const std::string& url : urls)
		{
			const std::string fileName = url.substr(url.find_last_of('/') + 1);
			std::ifstream f(targetPath + fileName);
			if (!f.good()) // download if file doesn't exist
			{
				std::string commandString = "curl " + url + " --output " + targetPath + fileName;
				system(commandString.c_str());
			}
		}
	}

	void Game::Initialize(int argc, char** argv)
	{
		FileUtils::DownloadFiles({
			"https://www.wikipedia.org/portal/wikipedia.org/assets/img/Wikipedia-logo-v2.png",
			"http://fonts.gstatic.com/s/abeezee/v11/mE5BOuZKGln_Ex0uYKpIaw.ttf",
			"https://fonts.gstatic.com/s/comicneue/v3/4UaHrEJDsxBrF37olUeDx63j5pN1MwI.ttf",
			"https://fonts.gstatic.com/s/nunitosans/v15/pe1kMImSLYBIv1o4X1M8cce4OdVisMz5nZRqy6cmmmU3t2FQWEAEOvV9wNvrwlNstMKW3Y6K5WMwXeVy3GboJ0kTHmqP92UnK_c.ttf"
			}, "examples/ui/");
		{
			textA = scene.CreateEntity();
			ScreenCoordinates& e_sc = textA.AddComponent<ScreenCoordinates>();
			e_sc.origin = { 0.0f, 0.0f };
			e_sc.offset = { 22.0f, 22.0f };
			Text& e_txt = textA.AddComponent<Text>();
			e_txt.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			e_txt.fontPath = "examples/ui/mE5BOuZKGln_Ex0uYKpIaw.ttf";
			e_txt.string = textStringA;
			e_txt.size = 50.0f;
			e_txt.alignmentH = ALIGNMENT_LEFT;
			e_txt.alignmentV = ALIGNMENT_TOP;
		}
		{
			textB = scene.CreateEntity();
			ScreenCoordinates& e_sc = textB.AddComponent<ScreenCoordinates>();
			e_sc.origin = { 1.0f, 1.0f };
			e_sc.offset = { -22.0f, -22.0f };
			Text& e_txt = textB.AddComponent<Text>();
			e_txt.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			e_txt.fontPath = "examples/ui/4UaHrEJDsxBrF37olUeDx63j5pN1MwI.ttf";
			e_txt.string = textStringB;
			e_txt.size = 50.0f;
			e_txt.alignmentH = ALIGNMENT_RIGHT;
			e_txt.alignmentV = ALIGNMENT_BOTTOM;
		}
		{
			textC = scene.CreateEntity();
			ScreenCoordinates& e_sc = textC.AddComponent<ScreenCoordinates>();
			e_sc.origin = { 0.5f, 0.75f };
			e_sc.offset = { 0.0f, 0.0f };
			Text& e_txt = textC.AddComponent<Text>();
			e_txt.color = { 1.0f, 1.0f, 1.0f, 1.0f };
			e_txt.fontPath = "examples/ui/pe1kMImSLYBIv1o4X1M8cce4OdVisMz5nZRqy6cmmmU3t2FQWEAEOvV9wNvrwlNstMKW3Y6K5WMwXeVy3GboJ0kTHmqP92UnK_c.ttf";
			e_txt.string = textStringC;
			e_txt.size = 50.0f;
			e_txt.alignmentH = ALIGNMENT_CENTER;
			e_txt.alignmentV = ALIGNMENT_CENTER;
		}

		{
			spriteTest = scene.CreateEntity();
			ScreenCoordinates& e_sc = spriteTest.AddComponent<ScreenCoordinates>();
			e_sc.origin = { 0.5f, 0.5f };
			e_sc.offset = { 0.0f, 0.0f };
			testSpriteBitmap = new Bitmap("examples/ui/Wikipedia-logo-v2.png");
			Sprite& e_spt = spriteTest.AddComponent<Sprite>(testSpriteBitmap);
			e_spt.alignmentH = ALIGNMENT_CENTER;
			e_spt.alignmentV = ALIGNMENT_CENTER;
		}
	}

	void Game::OnUpdate(float deltaTime, float time)
	{
	}

	void Game::ImGuiCall()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Text A"))
			{
				ImGui::DragFloat2("Origin", &textA.GetComponent<ScreenCoordinates>().origin.x, 0.01f);
				ImGui::DragInt2("Offset", &textA.GetComponent<ScreenCoordinates>().offset.x);
				ImGui::InputTextMultiline("String", textStringA, 256);
				ImGui::DragFloat("Size", &textA.GetComponent<Text>().size, 0.02f);
				ImGui::DragInt("Horizontal alignment", (int*)&textA.GetComponent<Text>().alignmentH, 0.2f);
				ImGui::DragInt("Vertical alignment", (int*)&textA.GetComponent<Text>().alignmentV, 0.2f);
				textA.GetComponent<Text>().alignmentH = Math::Mod(textA.GetComponent<Text>().alignmentH, 3);
				textA.GetComponent<Text>().alignmentV = Math::Mod(textA.GetComponent<Text>().alignmentV, 3);
				ImGui::ColorPicker4("Color", &textA.GetComponent<Text>().color.x);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Text B"))
			{
				ImGui::DragFloat2("Origin", &textB.GetComponent<ScreenCoordinates>().origin.x, 0.01f);
				ImGui::DragInt2("Offset", &textB.GetComponent<ScreenCoordinates>().offset.x);
				ImGui::InputTextMultiline("String", textStringB, 256);
				ImGui::DragFloat("Size", &textB.GetComponent<Text>().size, 0.02f);
				ImGui::DragInt("Horizontal alignment", (int*)&textB.GetComponent<Text>().alignmentH, 0.2f);
				ImGui::DragInt("Vertical alignment", (int*)&textB.GetComponent<Text>().alignmentV, 0.2f);
				textB.GetComponent<Text>().alignmentH = Math::Mod(textB.GetComponent<Text>().alignmentH, 3);
				textB.GetComponent<Text>().alignmentV = Math::Mod(textB.GetComponent<Text>().alignmentV, 3);
				ImGui::ColorPicker4("Color", &textB.GetComponent<Text>().color.x);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Text C"))
			{
				ImGui::DragFloat2("Origin", &textC.GetComponent<ScreenCoordinates>().origin.x, 0.01f);
				ImGui::DragInt2("Offset", &textC.GetComponent<ScreenCoordinates>().offset.x);
				ImGui::InputTextMultiline("String", textStringC, 256);
				ImGui::DragFloat("Size", &textC.GetComponent<Text>().size, 0.02f);
				ImGui::DragInt("Horizontal alignment", (int*)&textC.GetComponent<Text>().alignmentH, 0.2f);
				ImGui::DragInt("Vertical alignment", (int*)&textC.GetComponent<Text>().alignmentV, 0.2f);
				textC.GetComponent<Text>().alignmentH = Math::Mod(textC.GetComponent<Text>().alignmentH, 3);
				textC.GetComponent<Text>().alignmentV = Math::Mod(textC.GetComponent<Text>().alignmentV, 3);
				ImGui::ColorPicker4("Color", &textC.GetComponent<Text>().color.x);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Sprite"))
			{
				ImGui::DragFloat2("Origin", &spriteTest.GetComponent<ScreenCoordinates>().origin.x, 0.01f);
				ImGui::DragInt2("Offset", &spriteTest.GetComponent<ScreenCoordinates>().offset.x);
				ImGui::DragInt("Horizontal alignment", (int*)&spriteTest.GetComponent<Sprite>().alignmentH, 0.2f);
				ImGui::DragInt("Vertical alignment", (int*)&spriteTest.GetComponent<Sprite>().alignmentV, 0.2f);
				spriteTest.GetComponent<Sprite>().alignmentH = Math::Mod(spriteTest.GetComponent<Sprite>().alignmentH, 3);
				spriteTest.GetComponent<Sprite>().alignmentV = Math::Mod(spriteTest.GetComponent<Sprite>().alignmentV, 3);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void Game::Terminate()
	{
	}
}