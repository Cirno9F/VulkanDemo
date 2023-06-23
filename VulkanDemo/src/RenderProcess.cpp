#include "RenderProcess.h"
#include "Application.h"
#include "Context.h"
#include "SwapChain.h"

#include <glm/glm.hpp>

//TODO: 之后挪个地方
static vk::VertexInputAttributeDescription GetAttribute()
{
	vk::VertexInputAttributeDescription attribute;
	attribute.setBinding(0)
		.setFormat(vk::Format::eR32G32Sfloat)
		.setLocation(0)
		.setOffset(0);
	return attribute;
}

static vk::VertexInputBindingDescription GetBinding()
{
	vk::VertexInputBindingDescription binding;

	binding.setBinding(0)
		.setInputRate(vk::VertexInputRate::eVertex)
		.setStride(sizeof(glm::vec2));

	return binding;
}

static vk::DescriptorSetLayoutBinding GetUBOBinding()
{
	vk::DescriptorSetLayoutBinding binding;
	binding.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment)
		.setDescriptorCount(1);
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
	vk::RenderPassCreateInfo rpci;
	vk::AttachmentDescription ad;
	ad.setFormat(Context::s_Context->m_SwapChain->m_SwapChainInfo.Format.format)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setSamples(vk::SampleCountFlagBits::e1);
	rpci.setAttachments(ad);

	vk::AttachmentReference af;
	af.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setAttachment(0);
	vk::SubpassDescription sd;
	sd.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachments(af);
	rpci.setSubpasses(sd);

	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
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
		.setFrontFace(vk::FrontFace::eClockwise)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1);
	createInfo.setPRasterizationState(&rasterizationInfo);

	//6. multisample
	vk::PipelineMultisampleStateCreateInfo multisampleInfo;
	multisampleInfo.setSampleShadingEnable(false)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);
	createInfo.setPMultisampleState(&multisampleInfo);

	//7. test - stencil test, depth test

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

vk::DescriptorSetLayout RenderProcess::CreateSetLayout()
{
	vk::DescriptorSetLayoutCreateInfo createInfo;
	auto binding = GetUBOBinding();
	createInfo.setBindings(binding);
	return Context::s_Context->m_Device.createDescriptorSetLayout(createInfo);
}
