project "LightSource"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "$(SolutionDir)Walnut/vendor/imgui",
      "$(SolutionDir)Walnut/vendor/implot",
      "$(SolutionDir)Walnut/vendor/glfw/include",

      "$(SolutionDir)Walnut/Walnut/Source",
      "$(SolutionDir)Walnut/Walnut/Platform/GUI",

      --"%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.glmLS}",

      "$(SolutionDir)vendor/ChessAPI"
   }

    links
    {
        "Walnut",
        "ChessAPI"
    }

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      runtime "Debug"
      staticruntime "off"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      runtime "Release"
      staticruntime "off"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "WL_DIST" }
      runtime "Release"
      staticruntime "On"
      optimize "On"
      symbols "Off"