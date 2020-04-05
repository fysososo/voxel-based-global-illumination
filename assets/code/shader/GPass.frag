#version 430

in vec2 TexCoord;
in vec3 Position;
in mat3 TBN;


layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec4 gRoughness;
layout (location = 4) out vec4 gEmission;
layout (location = 5) out vec4 gSpecular;

layout (location = 0) uniform sampler2D NormalMap;
layout (location = 1) uniform sampler2D AlbedoMap;
layout (location = 2) uniform sampler2D RoughnessMap;
layout (location = 3) uniform sampler2D MetalnessMap;
layout (location = 4) uniform sampler2D EmissionMap;
layout (location = 5) uniform sampler2D SpecularMap;

uniform bool hasNormalMap;
uniform bool hasAlbedoMap;
uniform bool hasRoughnessMap;
uniform bool hasMetalnessMap;
uniform bool hasEmissionMap;
uniform bool hasSpecularMap;

uniform float metalness;
uniform float roughness;
uniform float shininess;
uniform vec3 albedo;
uniform vec3 emission;
uniform vec3 specular;
uniform float F0;
uniform float IOR;
uniform float KD;

vec3 EncodeNormal(vec3 normal)
{
    return normal * 0.5f + vec3(0.5f);
}

vec3 DecodeNormal(vec3 normal)
{
    return normal * 2.0f - vec3(1.0f);
}

void main(){
	//将贴图中的信息处理后存入各个GBuffer
	//……
	//法线
	if(hasNormalMap){
		lowp vec4 normal =  texture(NormalMap, TexCoord);
		vec3 N = normalize(TBN*DecodeNormal(normal.xyz));
		gNormal = N;
	}
	else{
		gNormal = normalize(TBN[2]);
	}

	//自發光
	if(hasEmissionMap){
		lowp vec4 emission_data =  texture(EmissionMap, TexCoord);
		gEmission = emission_data;
	}
	else{
		gEmission = vec4(emission, 1.0f);
	}

	//位置
	gPosition = Position;

	//ALBEDO
	if(hasAlbedoMap){
		lowp vec4 _albedo =  texture(AlbedoMap, TexCoord);
		gAlbedo = _albedo.xyz;
	}
	else{
		gAlbedo = albedo;
	}
	
	//金属度
	if(hasMetalnessMap){
		lowp vec4 _metalness = texture(MetalnessMap, TexCoord);
		gRoughness.g = _metalness.r;
	}
	else{
		gRoughness.g = metalness;
	}

	//镜面反射
	if(hasSpecularMap){
		lowp vec4 _specular = texture(SpecularMap, TexCoord);
		gSpecular = vec4(_specular.rgb * specular, max(shininess, 0.01f));;
	}
	else{
		gSpecular = vec4(specular, max(shininess, 0.01f));
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
}