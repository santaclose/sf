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
		"vendor/aobaker/*.cpp"
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
		"vendor/aobaker"
	}

	links 
	{ 
		"GLFW",
		"Glad",
		"opengl32.lib"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"SF_PLATFORM_WINDOWS",
			"GLFW_INCLUDE_NONE"
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