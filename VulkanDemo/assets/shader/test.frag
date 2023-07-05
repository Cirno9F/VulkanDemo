#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 vColor;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UBO
{
	vec3 color;
}ubo;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main()
{
	outColor = texture(tex, uv) * vec4(vColor, 1.0f);
}