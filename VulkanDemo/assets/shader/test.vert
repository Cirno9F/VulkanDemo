#version 450

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Color;

layout(binding = 1) uniform ModelViewProj
{
	mat4 model;
	mat4 view;
	mat4 proj;
}mvp;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 vColor;

void main(){
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4(Position, 1.0f);
	uv = TexCoord;
	vColor = Color;
}