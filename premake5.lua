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
	--include "vendor/tinygltf"
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
		"src/**.cpp",
		"user/**.h",
		"user/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/sfmg/*.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"vendor/GLFW/include",
		"vendor/Glad/include",
		"vendor/glm",
		"vendor/stb_image",
		"vendor/sfmg",
		"vendor/tinygltf",
		"vendor/assimp/include"
	}

	libdirs
	{
		"vendor/assimp/lib"
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
		links
		{
			"assimp-vc140-mtd"
		}

	filter "configurations:Release"
		defines "SF_RELEASE"
		runtime "Release"
		optimize "on"
		links
		{
			"assimp-vc140-mt"
		}