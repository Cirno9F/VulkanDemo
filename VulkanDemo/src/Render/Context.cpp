#include "Context.h"

Context* Context::s_Context = nullptr;
Ref<Shader> Context::s_TestShader = nullptr;


void Context::Init(uint32_t width, uint32_t height, const std::vector<const char*>& requiredExtensions, CreateSurfaceFunc createSurfaceFunc)
{
	s_Context = new Context(requiredExtensions, createSurfaceFunc);

	//shader
	s_TestShader = Shader::Create(Utils::ReadFile("assets/shader/vert.spv"), Utils::ReadFile("assets/shader/frag.spv"));

	s_Context->m_CommandManager = CreateScope<CommandManager>();
	s_Context->m_SwapChain = CreateScope<SwapChain>(width, height);
	s_Context->m_RenderProcess = CreateScope<RenderProcess>();
	s_Context->m_RenderProcess->InitRenderPass();
	s_Context->m_RenderProcess->InitLayout();
	s_Context->m_SwapChain->CreateFrameBuffers(width, height);
	s_Context->m_RenderProcess->InitPipeline(width,height);
	int maxFlightCount = 2;
	s_Context->m_DescriptorManager = CreateScope<DescriptorManager>(maxFlightCount);
	s_Context->m_Renderer = CreateScope<Renderer>(maxFlightCount);
}

void Context::Close()
{
	s_Context->m_Device.waitIdle();

	s_Context->m_Renderer = nullptr;
	s_Context->m_CommandManager = nullptr;
	s_Context->m_DescriptorManager = nullptr;

	//shader
	s_TestShader = nullptr;

	s_Context->m_RenderProcess = nullptr;
	s_Context->m_SwapChain = nullptr;

	delete s_Context;
	s_Context = nullptr;
}

Context::Context(const std::vector<const char*>& requiredExtensions, CreateSurfaceFunc createSurfaceFunc)
{
	CreateInstance(requiredExtensions);
	PickupPhysicalDevice();
	m_Surface = createSurfaceFunc(m_Instance);
	QueryQueueFamilyIndices();
	CreateDevice();
	GetQueues();
}

Context::~Context()
{
	m_Instance.destroySurfaceKHR(m_Surface);
	m_Device.destroy();
	m_Instance.destroy();
}

void Context::CreateInstance(const std::vector<const char*>& requiredExtensions)
{
	vk::InstanceCreateInfo createInfo;
	vk::ApplicationInfo appInfo;
	appInfo.setApiVersion(VK_API_VERSION_1_3);
	createInfo.setPApplicationInfo(&appInfo);
	m_Instance = vk::createInstance(createInfo);

	std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
	createInfo.setPEnabledLayerNames(layers)
		.setPEnabledExtensionNames(requiredExtensions);

	m_Instance = vk::createInstance(createInfo);
}

void Context::PickupPhysicalDevice()
{
	auto devices = m_Instance.enumeratePhysicalDevices();
	bool findPhysicalDevice = false;
	for (auto& device : devices)
	{
		auto feature = device.getFeatures();	
		if (feature.geometryShader)
		{
			m_PhysicalDevice = device;
			findPhysicalDevice = true;
			break;
		}
	}
	ASSERT_IFNOT(findPhysicalDevice, "Can not find physical device which enable geometryShader");

	LOG_TRACE("GPU: {0}", m_PhysicalDevice.getProperties().deviceName);
}

void Context::CreateDevice()
{
	std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	vk::DeviceCreateInfo createInfo;
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	float priorities = 1.0f;
	if (m_QueueFamilyIndices.PresentQueue.value() == m_QueueFamilyIndices.GraphicsQueue.value())
	{
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priorities)
			.setQueueCount(1)
			.setQueueFamilyIndex(m_QueueFamilyIndices.GraphicsQueue.value());
		queueCreateInfos.push_back(queueCreateInfo);
	}
	else
	{
		vk::DeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.setPQueuePriorities(&priorities)
			.setQueueCount(1)
			.setQueueFamilyIndex(m_QueueFamilyIndices.GraphicsQueue.value());
		queueCreateInfos.push_back(queueCreateInfo);
		queueCreateInfo.setPQueuePriorities(&priorities)
			.setQueueCount(1)
			.setQueueFamilyIndex(m_QueueFamilyIndices.PresentQueue.value());
		queueCreateInfos.push_back(queueCreateInfo);
	}
	createInfo.setQueueCreateInfos(queueCreateInfos)
		.setPEnabledExtensionNames(extensions);

	m_Device = m_PhysicalDevice.createDevice(createInfo);
}

void Context::GetQueues()
{
	m_GraphicsQueue = m_Device.getQueue(m_QueueFamilyIndices.GraphicsQueue.value(), 0);
	m_PresentQueue = m_Device.getQueue(m_QueueFamilyIndices.PresentQueue.value(), 0);
}

void Context::QueryQueueFamilyIndices()
{
	auto properties = m_PhysicalDevice.getQueueFamilyProperties();
	for (int i = 0; i < properties.size(); i++)
	{
		const auto& property = properties[i];
		if (property.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			m_QueueFamilyIndices.GraphicsQueue = i;
		}
		if (m_PhysicalDevice.getSurfaceSupportKHR(i, m_Surface))
		{
			m_QueueFamilyIndices.PresentQueue = i;
		}

		if (m_QueueFamilyIndices)
		{
			break;
		}
	}
}


