#version 450

layout(location = 0) in vec2 Position;

layout(binding = 1) uniform ModelViewProj
{
	mat4 model;
	mat4 view;
	mat4 proj;
}mvp;

void main(){
	gl_Position = mvp.proj * mvp.view * mvp.model * vec4(Position, 0.0f, 1.0f);
}