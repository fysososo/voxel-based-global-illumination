#version 430 core
#extension GL_ARB_explicit_uniform_location : enable
in vec2 TexCoord;
out vec4 fragColor;

//灯光
struct Light {
    vec3 position;
	vec3 color;
	float intensity;
};
const uint MAX_POINT_LIGHTS = 6;
uniform Light pointLight[MAX_POINT_LIGHTS];
uniform int lightCount;

//参数
uniform vec3 viewPos;
uniform float F0;
uniform float IOR;
uniform float KD;

//延迟渲染几何阶段结果
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gRoughness;
uniform sampler2D gMetalness;
uniform sampler2D gAbledo;

//体素3d纹理数据
uniform sampler3D voxelRadiance;
uniform sampler3D voxelNormal;
uniform sampler3D voxelIOR;

//体素数据
uniform float voxelScale;
uniform vec3 worldMinPoint;
uniform vec3 worldMaxPoint;
uniform int volumeDimension;

//BRDF
float D(vec3 N, vec3 H, float Roughness);
vec3 F(vec3 H, vec3 V, vec3 F0);
float G_direct(vec3 N, vec3 I, float Roughness);
float G_ibl(vec3 N, vec3 I, float Roughness);
vec3 f_Specular(vec3 N,vec3 L,vec3 H, vec3 V, float Roughness, float metalness, vec3 F0);
vec3 BRDF(vec3 albedo, vec3 N,vec3 L,vec3 H, vec3 V, float Roughness, float metalness, vec3 F0);
#define PI 3.1415926f

//视角锥追踪采样
vec3 conetrace(vec3 position, vec3 direction, vec3 aperture);

//视角锥采样阴影
vec3 conetraceShadow();

//采样漫反射光
//……

//采样镜面光
//……

//采样折射光
//……

//计算间接光照
//……

//计算直接光照
//……

void main(){
	vec3 pos = texture(gPosition, TexCoord).xyz;
	vec3 albedo = vec3(
		pow(texture(gAbledo, TexCoord).x, 2.2),
		pow(texture(gAbledo, TexCoord).y, 2.2),
		pow(texture(gAbledo, TexCoord).z, 2.2)
	);
	float roughness = texture(gRoughness, TexCoord).r;
	float metalness = texture(gMetalness, TexCoord).r;
	
	vec3 V = normalize(viewPos - pos);
	vec3 N = texture(gNormal, TexCoord).xyz;
	fragColor = vec4(0.0f);
	for(int i = 0; i <lightCount;i++){
		vec3 L = normalize(pointLight[i].position - pos);
		vec3 H = normalize(L+V);
		float distance = length(L);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = pointLight[i].color * attenuation * pointLight[i].intensity; 
		vec3 F = mix(vec3(F0), albedo, metalness);
		fragColor += vec4(BRDF(albedo, N, L, H, V, roughness, metalness, F)*max(dot(N, L),0.0f)*radiance,1.0f);
	}
}


vec3 F(vec3 H, vec3 V, vec3 F0){
	return F0 + (1-F0)*pow(1-max(dot(H, V), 0.0), 5);
}

float G_direct(vec3 N, vec3 I, float Roughness){
	float NI = max(dot(N,I),0);
	float k = pow(Roughness+1.0f,2)/8.0f;
	return NI/(NI*(1-k)+k);
}

float G_ibl(vec3 N, vec3 I, float Roughness){
	float NI = max(dot(N,I),0);
	float alpha = Roughness*Roughness;
	float R_2 = alpha*alpha;
	float k = R_2/8.0f;
	return NI/(NI*(1-k)+k);
}

vec3 f_Specular(vec3 N,vec3 L,vec3 H, vec3 V, float Roughness, float metalness, vec3 F0){
	float NV = max(dot(V,N), 0);
	float NL = max(dot(L,N),0);
	return (D(N,H,Roughness)*F(H,V,F0)*G_direct(N,L,Roughness)*G_direct(N,V,Roughness))/(4.0f*NV*NL + 0.001f);
}


float D(vec3 N, vec3 H, float Roughness){
	float NH = max(dot(N,H),0);
	float alpha = Roughness*Roughness;
	float R_2 = alpha*alpha;
	float denominator = NH*NH * (R_2 - 1) + 1;
	return R_2/(PI * denominator * denominator);
}

vec3 BRDF(vec3 albedo, vec3 N,vec3 L,vec3 H, vec3 V, float roughness, float metalness, vec3 F){
	vec3 diff = albedo/PI;
	vec3 spec = f_Specular(N, L, H, V, roughness, metalness, F);
	return (KD*diff+(1-KD)*(1-metalness)*spec);
}

vec3 conetrace(vec3 position, vec3 direction, vec3 aperture){
	return vec3(1.0f); 
}