-- premake5.lua
workspace "VulkanDemo"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "VulkanDemo"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

group "Dependencies"
   include "vendor/glfw"
group ""

group "Core"
	include "VulkanDemo"
group ""