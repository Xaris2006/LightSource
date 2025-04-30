project "ChessAPI"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"**.h",
		"**.cpp"
	}
	
	includedirs
	{
		"../xxHash/"
	}

	links
    {
		"xxHash"
	}

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		staticruntime "off"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		staticruntime "off"
		optimize "on"

    filter "configurations:Dist"
		runtime "Release"
		staticruntime "on"
		optimize "on"
        symbols "off"
