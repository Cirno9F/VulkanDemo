#version 450

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Color;

layout(binding = 1) uniform ViewProj
{
	mat4 view;
	mat4 proj;
}vp;

layout(push_constant) uniform PushConsts {
	mat4 model;
	mat4 imodel;
}cbuffer;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 vColor;
layout(location = 2) out vec3 normal;

void main(){
	gl_Position = vp.proj * vp.view * cbuffer.model * vec4(Position, 1.0f);
	uv = TexCoord;
	vColor = Color;
	normal = mat3(transpose(cbuffer.imodel)) * Normal;
}