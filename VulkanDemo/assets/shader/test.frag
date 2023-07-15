#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UBO
{
	vec3 color;
}ubo;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main()
{
	vec3 lightDir = normalize(vec3(1.0f,0.0f,0.0f));

	outColor = texture(tex, uv) * vec4(vColor, 1.0f) * dot(lightDir, normal);
}