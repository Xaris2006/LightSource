-- WalnutExternal.lua

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["glm"] = "../vendor/glm"
IncludeDir["spdlog"] = "../vendor/spdlog/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

--group "Dependencies"
--   include "Walnut/vendor/imgui"
--   include "Walnut/vendor/glfw"
--   include "Walnut/vendor/glad"
--   include "Walnut/vendor/yaml-cpp"
--group ""
--
--group "Core"
--    include "Walnut/Walnut/Build-Walnut.lua"
--group ""