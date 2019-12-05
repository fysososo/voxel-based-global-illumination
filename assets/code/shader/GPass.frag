#version 430 core
#extension GL_ARB_explicit_uniform_location : enable
in vec2 TexCoord;
in vec3 Position;
in mat3 TBN;


layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gRoughness;

layout (location = 0) uniform sampler2D NormalMap;
layout (location = 1) uniform sampler2D AlbedoMap;
layout (location = 2) uniform sampler2D RoughnessMap;
layout (location = 3) uniform sampler2D MetalnessMap;

uniform bool hasNormalMap;
uniform bool hasAlbedoMap;
uniform bool hasRoughnessMap;
uniform bool hasMetalnessMap;

uniform float metalness;
uniform float roughness;
uniform vec3 albedo;
uniform float F0;
uniform float IOR;
uniform float KD;

void main(){
	//将贴图中的信息处理后存入各个GBuffer
	//……
	//法线
	if(hasNormalMap){
		lowp vec4 normal =  texture(NormalMap, TexCoord);
		normal = normal*2.0f - 1.0f;
		vec3 N = normalize(TBN*normal.xyz);
		gNormal = N;
	}
	else{
		gNormal = normalize(TBN[2]);
	}

	//位置
	gPosition = Position;

	//ALBEDO
	if(hasAlbedoMap){
		lowp vec4 _albedo =  texture(AlbedoMap, TexCoord);
		gAlbedoSpec.rgb = _albedo.xyz;
	}
	else{
		gAlbedoSpec.rgb = albedo;
	}
	
	//金属度
	if(hasMetalnessMap){
		lowp vec4 _metalness = texture(MetalnessMap, TexCoord);
		gRoughness.g = _metalness.r;
	}
	else{
		gRoughness.g = metalness;
	}

	//粗糙度
	if(hasRoughnessMap){
		lowp vec4 _roughness =  texture(RoughnessMap, TexCoord);
		gRoughness.r = _roughness.r;
	}
	else{
		gRoughness.r = roughness;
	}
	gRoughness.b = F0;
	gRoughness.a = IOR;
	gAlbedoSpec.a = KD;
}