project "VulkanDemo"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp", "../vendor/stb_image/**.h", "../vendor/stb_image/**.cpp",  "../vendor/tinyobjloader/**.h"}

   includedirs
   {
      "../vendor/glfw/include",
      "../vendor/stb_image",
      "../vendor/glm",
      "../vendor/spdlog/include",
      "../vendor/tinyobjloader",
      "../vendor/imgui",
      "%{IncludeDir.VulkanSDK}",
   }

   links
   {
       "GLFW",
       "ImGui",
       "%{Library.Vulkan}",
   }

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"

   filter "configurations:Debug"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"