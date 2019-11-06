#version 430 core
#extension GL_ARB_explicit_uniform_location : enable
in vec2 TexCoord;
in vec3 Position;
in mat3 TBN;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedoSpec;
layout (location = 3) out float gRoughness;
layout (location = 4) out float gMetalness;

layout (location = 0) uniform sampler2D NormalMap;
layout (location = 1) uniform sampler2D AlbedoMap;
layout (location = 2) uniform sampler2D RoughnessMap;
layout (location = 3) uniform sampler2D MetalnessMap;

void main(){
	//将贴图中的信息处理后存入各个GBuffer
	//……
	//法线
	lowp vec4 normal =  texture(NormalMap, TexCoord);
	normal = normal*2.0f - 1.0f;
	vec3 N = normalize(TBN*normal.xyz);
	gNormal = N;

	//位置
	gPosition = Position;

	//ALBEDO
	lowp vec4 albedo =  texture(AlbedoMap, TexCoord);
	gAlbedoSpec = albedo.xyz;

	//粗糙度
	lowp vec4 roughness =  texture(RoughnessMap, TexCoord);
	gRoughness = roughness.r;

	//金属度
	lowp vec4 metalness =  texture(MetalnessMap, TexCoord);
	gMetalness = metalness.r;
}