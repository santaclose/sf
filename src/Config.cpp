#include "Config.h"

#include <iostream>
#include <fstream>
#include <json.hpp>

namespace sf::Config
{
    std::string name = "sfgame";
    glm::uvec2 windowSize = { 1280, 720 };
    uint32_t msaaCount = 4;
    glm::vec3 clearColor = { 1.0f, 1.0f, 1.0f };
    bool fullscreen = false;
    bool imguiBarEnabled = true;
    bool cursorEnabled = true;
    GLFWwindow* window;
}

void sf::Config::LoadFromFile(const std::string& filePath)
{
    std::ifstream i(filePath);
    if (!i.good())
    {
        std::cout << "[Config] Config file not found\n";
        return;
    }
    nlohmann::json j;
    i >> j;
    if (j.find("name") != j.end())
        name = j["name"];
    if (j.find("windowWidth") != j.end() && j.find("windowHeight") != j.end())
        windowSize = { j["windowWidth"], j["windowHeight"] };
    if (j.find("msaaCount") != j.end())
        msaaCount = j["msaaCount"];
    if (j.find("fullscreen") != j.end())
        fullscreen = j["fullscreen"];
    if (j.find("clearColorR") != j.end() && j.find("clearColorG") != j.end() && j.find("clearColorB") != j.end())
        clearColor = { j["clearColorR"], j["clearColorG"], j["clearColorB"] };
    if (j.find("imguiBarEnabled") != j.end())
        imguiBarEnabled = j["imguiBarEnabled"];
    if (j.find("cursorEnabled") != j.end())
        cursorEnabled = j["cursorEnabled"];
    std::cout << "[Config] Config file loaded\n";
}

void sf::Config::UpdateWindow(GLFWwindow* window) { sf::Config::window = window; }
void sf::Config::UpdateWindowSize(int width, int height) { windowSize = { width, height }; }

void sf::Config::SetImGuiBarEnabled(bool value) { imguiBarEnabled = value; }
void sf::Config::SetFullscreen(bool value)
{
    if (fullscreen == value)
        return;

    fullscreen = value;
    static int xpos, ypos, width, height, monitorCount, monitorxpos, monitorypos;
    if (fullscreen)
    {
        // backup window position and window size
        glfwGetWindowPos(window, &xpos, &ypos);
        glfwGetWindowSize(window, &width, &height);

        // get resolution of monitor
        GLFWmonitor* targetMonitor = glfwGetPrimaryMonitor();
        GLFWmonitor** monitorList = glfwGetMonitors(&monitorCount);
        float minMonitorDistance = std::numeric_limits<float>::infinity();
        for (int i = 0; i < monitorCount; i++)
        {
            glfwGetMonitorPos(monitorList[i], &monitorxpos, &monitorypos);
            float distance = glm::distance(glm::vec2(xpos, ypos), glm::vec2(monitorxpos, monitorypos));
            if (distance < minMonitorDistance)
            {
                minMonitorDistance = distance;
                targetMonitor = monitorList[i];
            }
        }
        const GLFWvidmode* mode = glfwGetVideoMode(targetMonitor);

        // switch to full screen
        glfwSetWindowMonitor(window, targetMonitor, 0, 0, mode->width, mode->height, 0);
    }
    else
    {
        // restore last window size and position
        glfwSetWindowMonitor(window, nullptr, xpos, ypos, width, height, 0);
    }
}

void sf::Config::SetCursorEnabled(bool value)
{
    if (cursorEnabled == value)
        return;
    cursorEnabled = value;
    glfwSetInputMode(window, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

const std::string& sf::Config::GetName() { return name; }
const glm::uvec2& sf::Config::GetWindowSize() { return windowSize; }
const uint32_t sf::Config::GetMsaaCount() { return msaaCount; }
const glm::vec3& sf::Config::GetClearColor() { return clearColor; }
bool sf::Config::GetFullscreen() { return fullscreen; }
bool sf::Config::GetImGuiBarEnabled() { return imguiBarEnabled; }
bool sf::Config::GetCursorEnabled() { return cursorEnabled; }
