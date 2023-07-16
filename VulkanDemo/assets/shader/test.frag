#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 normal;
layout(location = 5) in vec4 world;

//TODO: 放到uniform buffer中去
layout(location = 3) in vec3 lightDir;
layout(location = 4) in vec3 cameraPos;


layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform Material
{
	vec3 albedo;
	float roughness;
	float metallic;
}mat;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main()
{
	vec3 v = normalize(cameraPos - vec3(world));
	vec3 l = normalize(-lightDir);
	vec3 h = normalize(v+l);
	float diffuse = max(dot(l, normal), 0.0f);
	float specular = max(pow(dot(h, normal), 3.0f) ,0.0f);
	outColor = vec4(mat.albedo * (diffuse + specular), 1.0f);
}