#include "RenderProcess.h"
#include "Application.h"
#include "Context.h"

RenderProcess::RenderProcess()
{
}

RenderProcess::RenderProcess(uint32_t width, uint32_t height)
{
	InitPipeline(width, height);
}

RenderProcess::~RenderProcess()
{
	DestroyPipeline();
}

void RenderProcess::InitPipeline(uint32_t width, uint32_t height)
{
	vk::GraphicsPipelineCreateInfo createInfo;

	//1. Vertex Input
	vk::PipelineVertexInputStateCreateInfo inputState;
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

	//layout
	//renderpass

	auto result = Context::s_Context->m_Device.createGraphicsPipeline(nullptr, createInfo);
	ASSERT_IFNOT(result.result == vk::Result::eSuccess, "Create graphics pipeline failed!");
	m_Pipeline = result.value;

}

void RenderProcess::DestroyPipeline()
{
	Context::s_Context->m_Device.destroyPipeline(m_Pipeline);
}
