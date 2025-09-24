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
	include "vendor/glfw"
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
		-- one example project
		"examples/ui/**.h",
		"examples/ui/**.hpp",
		"examples/ui/**.cpp",

		"src/**.h",
		"src/**.hpp",
		"src/**.cpp",
		"user/**.h",
		"user/**.hpp",
		"user/**.cpp",
		"vendor/stb/**.h",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/sfmg/*.cpp",
		"vendor/nanosvg/*.cpp",
		"vendor/sebtext/*.cpp",
		"vendor/vendor.cpp"
	}

	defines
	{
		"GLFW_INCLUDE_NONE",
		"_CRT_SECURE_NO_WARNINGS",
		"SF_USE_OPENGL"
	}

	includedirs
	{
		"src",
		"vendor/glfw/include",
		"vendor/Glad/include",
		"vendor/glm",
		"vendor/stb",
		"vendor/sfmg",
		"vendor/tinygltf",
		"vendor/gli/gli",
		"vendor/entt/src/entt",
		"vendor/nanosvg",
		"vendor/imgui",
		"vendor/sebtext",
		"vendor/orangeduck"
	}

	links 
	{ 
		"glfw",
		"Glad",
		"ImGui",
		-- "opengl32"
	}

	filter "system:Windows"
		systemversion "latest"

		defines
		{
			"SF_PLATFORM_WINDOWS"
		}

	filter "system:Unix"
		system "linux"
		systemversion "latest"
		defines
		{
			"SF_PLATFORM_LINUX"
		}

	filter "configurations:Debug"
		defines "SF_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SF_RELEASE"
		runtime "Release"
		optimize "on"

	filter { "action:gmake" }
		buildoptions { "-fopenmp" }
		linkoptions { "-fopenmp" }

	filter { "action:vs2022" }
		buildoptions { "/openmp" }