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

bool hasMap[4];

uniform float metalness;
uniform float roughness;
uniform vec3 albedo;

void main(){
	//����ͼ�е���Ϣ�����������GBuffer
	//����
	//����
	if(hasMap[0]){
		lowp vec4 normal =  texture(NormalMap, TexCoord);
		normal = normal*2.0f - 1.0f;
		vec3 N = normalize(TBN*normal.xyz);
		gNormal = N;
	}
	else{
		gNormal = normalize(TBN*TBN[2]);
	}

	//λ��
	gPosition = Position;

	//ALBEDO
	if(hasMap[2]){
		lowp vec4 _albedo =  texture(AlbedoMap, TexCoord);
		gAlbedoSpec = _albedo.xyz;
	}
	else{
		gAlbedoSpec = albedo.xyz;
	}
	//�ֲڶ�
	if(hasMap[3]){
		lowp vec4 _roughness =  texture(RoughnessMap, TexCoord);
		gRoughness = _roughness.r;
	}
	else{
		gRoughness = roughness;
	}

	//������
	if(hasMap[1]){
		lowp vec4 _metalness = texture(MetalnessMap, TexCoord);
		gMetalness = _metalness.r;
	}
	else{
		gMetalness = metalness;
	}
}