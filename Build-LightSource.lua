-- premake5.lua
workspace "LightSource"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "LightSource"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Build-Walnut-External.lua"
include "LightSource/Build-LightSource-App.lua"
include "LightSource-Lobby/Build-LightSource-Lobby-App.lua"