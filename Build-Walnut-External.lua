-- WalnutExternal.lua

VULKAN_SDK = os.getenv("VULKAN_SDK")
OPENSSL = "C:/Program Files/OpenSSL-Win64"--os.getenv("OPENSSL_ROOT_DIR") or os.getenv("OPENSSL_DIR")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["OpenSSL"] = "%{OPENSSL}/include"
IncludeDir["glm"] = "../vendor/glm"
IncludeDir["spdlog"] = "../vendor/spdlog/include"
IncludeDir["glmLS"] = "$(SolutionDir)Walnut/vendor/glm"
IncludeDir["spdlogLs"] = "$(SolutionDir)Walnut/vendor/spdlog/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["OpenSSL"] = "%{OPENSSL}/lib/VC/x64/MD"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["OpenSSL_Crypto"] = "%{LibraryDir.OpenSSL}/libcrypto.lib"
Library["OpenSSL_Ssl"] = "%{LibraryDir.OpenSSL}/libssl.lib"

group "Dependencies"
   include "Walnut/vendor/imgui"
   include "Walnut/vendor/implot"
   include "Walnut/vendor/glfw"
   include "Walnut/vendor/glad"
   include "Walnut/vendor/yaml-cpp"
group ""

group "Core"
    include "Walnut/Walnut/Build-Walnut.lua"
group ""