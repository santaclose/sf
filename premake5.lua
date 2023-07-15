VULKAN_SDK = os.getenv("VULKAN_SDK")
workspace "sf"
	architecture "x64"
	startproject "sf"

	configurations
	{
		"Debug",
		"Release"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Projects
group "Dependencies"
	include "vendor/GLFW"
	include "vendor/Glad"
	include "vendor/imgui"

group ""

project "sf"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
		"user/**.h",
		"user/**.hpp",
		"user/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/sfmg/*.cpp",
		"vendor/aobaker/*.cpp",
		"vendor/nanosvg/*.cpp",
		"vendor/load.cpp",

		"examples/pbr/*.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"src",
		"vendor/GLFW/include",
		"vendor/Glad/include",
		"vendor/glm",
		"vendor/stb_image",
		"vendor/sfmg",
		"vendor/tinygltf",
		"vendor/gli/gli",
		"vendor/aobaker",
		"vendor/entt",
		"vendor/nanosvg",
		"vendor/imgui",
		"%{VULKAN_SDK}/Include"
	}

	libdirs
	{
		"%{VULKAN_SDK}/Lib"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib",
		"vulkan-1.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SF_PLATFORM_WINDOWS",
			"SF_USE_OPENGL"
		}

	filter "configurations:Debug"
		defines "SF_DEBUG"
		runtime "Debug"
		symbols "on"
		buildoptions { "/openmp" }

	filter "configurations:Release"
		defines "SF_RELEASE"
		runtime "Release"
		optimize "on"
		buildoptions { "/openmp" }