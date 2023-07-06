#include "RenderProcess.h"
#include "../Application.h"
#include "Context.h"
#include "SwapChain.h"

#include <glm/glm.hpp>

//TODO: 之后挪个地方
static std::vector<vk::VertexInputAttributeDescription> GetAttribute()
{
	std::vector<vk::VertexInputAttributeDescription> attribute;
	attribute.resize(3);
	attribute[0].setBinding(0)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setLocation(0)
		.setOffset(0);
	attribute[1].setBinding(0)
		.setFormat(vk::Format::eR32G32Sfloat)
		.setLocation(1)
		.setOffset(offsetof(VertexInput, TexCoord));
	attribute[2].setBinding(0)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setLocation(2)
		.setOffset(offsetof(VertexInput, Color));
	return attribute;
}

static vk::VertexInputBindingDescription GetBinding()
{
	vk::VertexInputBindingDescription binding;

	binding.setBinding(0)
		.setInputRate(vk::VertexInputRate::eVertex)
		.setStride(sizeof(VertexInput));

	return binding;
}


RenderProcess::RenderProcess()
{
}

RenderProcess::RenderProcess(uint32_t width, uint32_t height)
{
	InitLayout();
	InitRenderPass();
	InitPipeline(width, height);
}

RenderProcess::~RenderProcess()
{
	auto& device = Context::s_Context->m_Device;
	for(auto& layout : m_SetLayout)
		device.destroyDescriptorSetLayout(layout);
	device.destroyRenderPass(m_RenderPass);
	device.destroyPipelineLayout(m_PipelineLayout);
	device.destroyPipeline(m_Pipeline);
}

void RenderProcess::InitLayout()
{
	//定义uniform变量的布局
	vk::PipelineLayoutCreateInfo createInfo;
	m_SetLayout = CreateSetLayout();
	createInfo.setSetLayouts(m_SetLayout);
	m_PipelineLayout = Context::s_Context->m_Device.createPipelineLayout(createInfo);
}

void RenderProcess::InitRenderPass()
{
	vk::AttachmentDescription colorAttachment;
	colorAttachment.setFormat(Context::s_Context->m_SwapChain->m_SwapChainInfo.Format.format)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setSamples(vk::SampleCountFlagBits::e1);
	vk::AttachmentReference colorAttachmentRef;
	colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setAttachment(0);

	vk::AttachmentDescription depthAttachment;
	depthAttachment.setFormat(Context::s_Context->m_SwapChain->m_DepthTexture->GetFormat())
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setSamples(vk::SampleCountFlagBits::e1);
	vk::AttachmentReference depthAttachmentRef;
	depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setAttachment(1);


	vk::SubpassDescription sd;
	sd.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachments(colorAttachmentRef)
		.setPDepthStencilAttachment(&depthAttachmentRef);


	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);


	std::array<vk::AttachmentDescription, 2> atttachments{ colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo rpci;
	rpci.setAttachments(atttachments);
	rpci.setSubpasses(sd);
	rpci.setDependencies(dependency);
	m_RenderPass = Context::s_Context->m_Device.createRenderPass(rpci);
}

void RenderProcess::InitPipeline(uint32_t width, uint32_t height)
{
	vk::GraphicsPipelineCreateInfo createInfo;

	//1. Vertex Input
	vk::PipelineVertexInputStateCreateInfo inputState;
	auto attribute = GetAttribute();
	auto binding = GetBinding();
	inputState.setVertexBindingDescriptions(binding)
		.setVertexAttributeDescriptions(attribute);
	createInfo.setPVertexInputState(&inputState);

	//2. Vertex Assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.setPrimitiveRestartEnable(false)
		.setTopology(vk::PrimitiveTopology::eTriangleList);
	createInfo.setPInputAssemblyState(&inputAssembly);

	//3. Shader
	createInfo.setStages(Context::s_TestShader->m_Stages);

	//4. Viewport
	vk::PipelineViewportStateCreateInfo viewportStateInfo;
	vk::Viewport viewport(0, 0, width, height, 0, 1);
	viewportStateInfo.setViewports(viewport);
	vk::Rect2D rect({ 0,0 }, { width,height });
	viewportStateInfo.setScissors(rect);
	createInfo.setPViewportState(&viewportStateInfo);

	//5. Rasterization
	vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
	rasterizationInfo.setRasterizerDiscardEnable(false)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1);
	createInfo.setPRasterizationState(&rasterizationInfo);

	//6. multisample
	vk::PipelineMultisampleStateCreateInfo multisampleInfo;
	multisampleInfo.setSampleShadingEnable(false)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);
	createInfo.setPMultisampleState(&multisampleInfo);

	//7. test - stencil test, depth test
	vk::PipelineDepthStencilStateCreateInfo depthInfo;
	depthInfo.setDepthTestEnable(true)
		.setDepthWriteEnable(true)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(false)
		.setMinDepthBounds(0.0f)
		.setMaxDepthBounds(1.0f)
		.setStencilTestEnable(false)
		.setFront({})
		.setBack({});
	createInfo.setPDepthStencilState(&depthInfo);

	//8. color blending
	vk::PipelineColorBlendStateCreateInfo blendInfo;
	vk::PipelineColorBlendAttachmentState attachmentState;
	attachmentState.setBlendEnable(false)
		.setColorWriteMask(vk::ColorComponentFlagBits::eA |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eR );
	blendInfo.setLogicOpEnable(false)
		.setAttachments(attachmentState);
	createInfo.setPColorBlendState(&blendInfo);

	//layout & renderpass
	createInfo.setRenderPass(m_RenderPass)
		.setLayout(m_PipelineLayout);


	auto result = Context::s_Context->m_Device.createGraphicsPipeline(nullptr, createInfo);
	ASSERT_IFNOT(result.result == vk::Result::eSuccess, "Create graphics pipeline failed!");
	m_Pipeline = result.value;

}

std::vector<vk::DescriptorSetLayout> RenderProcess::CreateSetLayout()
{
	std::vector<vk::DescriptorSetLayout> layouts;
	layouts.resize(2);

	vk::DescriptorSetLayoutCreateInfo createInfo;
	std::array<vk::DescriptorSetLayoutBinding, 2> bindings;
	bindings[0].setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment)
		.setDescriptorCount(1);
	bindings[1].setBinding(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setDescriptorCount(1);
	createInfo.setBindings(bindings);
	layouts[0] = Context::s_Context->m_Device.createDescriptorSetLayout(createInfo);


	vk::DescriptorSetLayoutBinding binding;
	binding.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment)
		.setDescriptorCount(1);
	createInfo.setBindings(binding);
	layouts[1] = Context::s_Context->m_Device.createDescriptorSetLayout(createInfo);
	return layouts;
}
