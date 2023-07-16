#version 450

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Color;

layout(binding = 1) uniform CommonBufferInfo
{
	mat4 view;
	mat4 proj;
	vec3 lightDir;
	vec3 cameraPos;
}commonBuffer;

layout(push_constant) uniform PushConsts {
	mat4 model;
	mat4 imodel;
}constBuffer;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 vColor;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec3 lightDir;
layout(location = 4) out vec3 cameraPos;
layout(location = 5) out vec4 world;

void main(){
	world = constBuffer.model * vec4(Position, 1.0f);
	gl_Position = commonBuffer.proj * commonBuffer.view * world;
	uv = TexCoord;
	vColor = Color;
	normal = mat3(transpose(constBuffer.imodel)) * Normal;
	lightDir = normalize(commonBuffer.lightDir);
	cameraPos = commonBuffer.cameraPos;
}