#pragma once

#include "Base.h"
#include "SwapChain.h"

#include <vulkan/vulkan.hpp>
#include <memory>
#include <optional>
#include <functional>

using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;

class Context
{
public:
	struct QueueFamilyIndices {
		std::optional<uint32_t> GraphicsQueue;
		std::optional<uint32_t> PresentQueue;

		operator bool() const {
			return GraphicsQueue.has_value() && PresentQueue.has_value();
		}
	};

	static void Init(uint32_t width, uint32_t height, const std::vector<const char*>& requiredExtensions, CreateSurfaceFunc createSurfaceFunc);
	static void Close();
public:
	static Context* s_Context;

	vk::Instance m_Instance;
	vk::PhysicalDevice m_PhysicalDevice;
	vk::Device m_Device;
	vk::Queue m_GraphicsQueue;
	vk::Queue m_PresentQueue;
	vk::SurfaceKHR m_Surface;
	Scope<SwapChain> m_SwapChain;
	QueueFamilyIndices m_QueueFamilyIndices;
private:
	Context(const std::vector<const char*>& requiredExtensions, CreateSurfaceFunc createSurfaceFunc);
	~Context();

	void CreateInstance(const std::vector<const char*>& requiredExtensions);
	void PickupPhysicalDevice();
	void CreateDevice();
	void GetQueues();

	void QueryQueueFamilyIndices();
};
