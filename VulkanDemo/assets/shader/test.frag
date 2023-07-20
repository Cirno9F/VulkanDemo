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
	float reflectance;
}mat;

layout(set = 1, binding = 0) uniform sampler2D tex;


//math
float pow2(float x)
{
	return x * x;
}

//BRDF
#define PI 3.1415926
//Normal Distribution Function
float D_GGX(float NoH, float roughness)
{
	float a = NoH * roughness;
	float k = roughness / (1.0f - pow2(NoH) + pow2(a));
	return k * k * (1.0 / PI);
}
//Geometric Shadowing
float V_SmithGGXCorrelated(float NoV, float NoL, float roughness)
{
	float a2 = roughness * roughness;
	float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
	float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
	return 0.5f / (GGXV + GGXL);
}
//Fresnel
vec3 F_Schlick(float u, vec3 f0, float f90)
{
	return f0 + (vec3(f90)-f0)*pow(1.0f-u,5.0f);
}
float Fd_Lambert()
{
	return 1.0f / PI;
}


void main()
{
	vec3 n = normalize(normal);
	vec3 v = normalize(cameraPos - vec3(world));
	vec3 l = normalize(-lightDir);
	vec3 h = normalize(v+l);

	float NoV = clamp(dot(n,v),0.0f,1.0f)+1e-5;
	float NoL = clamp(dot(n,l),0.0f,1.0f);
	float NoH = clamp(dot(n,h),0.0f,1.0f);
	float LoH = clamp(dot(l,h),0.0f,1.0f);

	float roughness = mat.roughness;

	//most dielectric material's reflectance is 4%
	float reflectance = mat.reflectance;
	float D = D_GGX(NoH,roughness);
	vec3 f0 = 0.16 * reflectance * reflectance * (1 - mat.metallic) + mat.albedo * mat.metallic;
	float f90 = 1.0f;
	vec3 F = F_Schlick(LoH, f0, f90);
	float V = V_SmithGGXCorrelated(NoV,NoL,roughness);

	vec3 Fr = (D * V) * F;
	vec3 Fd = mat.albedo * (1 - mat.metallic) * Fd_Lambert();

	vec3 lightColor = vec3(1.0f);
	outColor = vec4(lightColor * (Fr + Fd) * NoL, 1.0f); 
}