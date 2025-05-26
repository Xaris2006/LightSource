project "Update"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"

   files { "**.h", "**.cpp" }

   includedirs
   {
      "$(SolutionDir)Walnut/vendor/glfw/include",
      "$(SolutionDir)Walnut/vendor/glad/include",
      --"%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.glmLS}"
   }

    links
    {
        "glad",
        "glfw"
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