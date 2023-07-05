#include "Shader.h"
#include "Context.h"

Ref<Shader> Shader::Create(const std::string& vertexSource, const std::string& fragSource)
{
	return CreateRef<Shader>(vertexSource, fragSource);
}

Shader::~Shader()
{
	auto& device = Context::s_Context->m_Device;
	device.destroyShaderModule(m_VertexShader);
	device.destroyShaderModule(m_FragmentShader);
}

void Shader::InitStages()
{	
	m_Stages.resize(2);
	m_Stages[0].setStage(vk::ShaderStageFlagBits::eVertex)
		.setModule(m_VertexShader)
		.setPName("main");
	m_Stages[1].setStage(vk::ShaderStageFlagBits::eFragment)
		.setModule(m_FragmentShader)
		.setPName("main");
}

Shader::Shader(const std::string& vertexSource, const std::string& fragSource)
{
	vk::ShaderModuleCreateInfo vsCreateInfo;
	vsCreateInfo.codeSize = vertexSource.size();
	vsCreateInfo.pCode = (uint32_t*)vertexSource.data();
	m_VertexShader = Context::s_Context->m_Device.createShaderModule(vsCreateInfo);
	
	vk::ShaderModuleCreateInfo fsCreateInfo;
	fsCreateInfo.codeSize = fragSource.size();
	fsCreateInfo.pCode = (uint32_t*)fragSource.data();
	m_FragmentShader = Context::s_Context->m_Device.createShaderModule(fsCreateInfo);

	InitStages();
}
