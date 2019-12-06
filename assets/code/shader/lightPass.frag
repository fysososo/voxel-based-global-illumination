#version 430 core
#extension GL_ARB_explicit_uniform_location : enable
in vec2 TexCoord;
out vec4 fragColor;

//�ƹ�
struct Light {
    vec3 position;
	vec3 color;
	float intensity;
};
const uint MAX_POINT_LIGHTS = 6;
uniform Light pointLight[MAX_POINT_LIGHTS];
uniform int lightCount;

//����
uniform vec3 viewPos;
uniform float coneShadowTolerance;
uniform float samplingFactor = 0.5f;

//�ӳ���Ⱦ���ν׶ν��
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gRoughness;
uniform sampler2D gMetalness;
uniform sampler2D gAbledo;

//����3d��������
uniform sampler3D voxelRadiance;
uniform sampler3D voxelTexMipmap[6];
uniform sampler3D voxelNormal;
uniform sampler3D voxelIOR;

//��������
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
vec3 BRDF(vec3 albedo, vec3 N,vec3 L,vec3 H, vec3 V, float Roughness, float metalness, vec3 F0, float KD);
#define PI 3.1415926f

//�Ƿ��볡���ཻ
bool IsIntersectWithWorldAABB(vec3 o, vec3 d, out float leave, out float enter);

//��������仯Ϊ��������
vec3 WorldToVoxel(vec3 position);

//�������Բ���
vec4 AnistropicSample(vec3 coord, vec3 weight, uvec3 face, float lod);

//�ӽ�׶׷�ٲ���
vec3 conetrace(vec3 position, vec3 direction, vec3 aperture);

//�ӽ�׶������Ӱ
float conetraceShadow(vec3 position, vec3 direction, float aperture, float maxTracingDistance);

//�����������
//����

//���������
//����

//���������
//����

//�����ӹ���
//����

//����ֱ�ӹ���
//����

void main(){
	vec3 pos = texture(gPosition, TexCoord).xyz;
	vec4 albedo = vec4(
		pow(texture(gAbledo, TexCoord).x, 2.2),
		pow(texture(gAbledo, TexCoord).y, 2.2),
		pow(texture(gAbledo, TexCoord).z, 2.2),
		texture(gAbledo, TexCoord).a
	);
	vec4 roughness = texture(gRoughness, TexCoord);
	float metalness = roughness.g;
	float F0 = roughness.b;
	
	vec3 V = normalize(viewPos - pos);
	vec3 N = texture(gNormal, TexCoord).xyz;
	fragColor = vec4(0.0f);
	for(int i = 0; i <lightCount;i++){
		vec3 L = normalize(pointLight[i].position - pos);
		vec3 H = normalize(L+V);
		float distance = length(L);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = pointLight[i].color * attenuation * pointLight[i].intensity; 
		vec3 F = mix(vec3(F0), albedo.rgb, metalness);
		float dis = length(pointLight[i].position - pos);
		//fragColor = vec4(max(0.0f, conetraceShadow(pos, L, 0.03f, volumeDimension*voxelScale)));
		fragColor += vec4
		(
			BRDF(albedo.rgb, N, L, H, V, roughness.r, metalness, F, albedo.a)
			*max(dot(N, L),0.0f)
			*radiance
			*max(0.0f, conetraceShadow(pos, L, 0.02f, dis))
			*(1.0f/(0.01f+0.05f*dis+0.1* dis*dis))
			,1.0f
		);
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


vec3 conetrace(vec3 position, vec3 direction, vec3 aperture){
	return vec3(1.0f); 
}

bool IsIntersectWithWorldAABB(vec3 o, vec3 d, out float leave, out float enter){
	vec3 tMin = (worldMinPoint - o) / d;
	vec3 tMax = (worldMaxPoint - o) / d;

	vec3 vMax = max(tMin, tMax);
	vec3 vMin = min(tMin, tMax);

	leave = min(vMax.x, min(vMax.y, vMax.z));
	enter = max (max (vMin.x, 0.0), max (vMin.y, vMin.z));
	return leave > enter;
}

vec3 BRDF(vec3 albedo, vec3 N,vec3 L,vec3 H, vec3 V, float roughness, float metalness, vec3 F, float KD){
	vec3 diff = albedo/PI;
	vec3 spec = f_Specular(N, L, H, V, roughness, metalness, F);
	return (KD*diff+(1-KD)*(1-metalness)*spec);
}

vec3 WorldToVoxel(vec3 position){
	vec3 voxelPos = position - worldMinPoint;
    return voxelPos / (voxelScale*volumeDimension);
}

vec4 AnistropicSample(vec3 coord, vec3 weight, uvec3 face, float lod){
	float anisoLevel = max(lod - 1.0f, 0.0f);
	vec4 anisoSample = weight.x * textureLod(voxelTexMipmap[face.x], coord, anisoLevel)
                     + weight.y * textureLod(voxelTexMipmap[face.y], coord, anisoLevel)
                     + weight.z * textureLod(voxelTexMipmap[face.z], coord, anisoLevel);
	if(lod < 1.0f)
    {
        vec4 baseColor = texture(voxelRadiance, coord);
        anisoSample = mix(baseColor, anisoSample, clamp(lod, 0.0f, 1.0f));
    }
    return anisoSample;               
}

float conetraceShadow(vec3 position, vec3 direction, float aperture, float maxTracingDistance){
	//ѡ�������
	uvec3 visibleFace;
    visibleFace.x = (direction.x < 0.0) ? 0 : 1;
    visibleFace.y = (direction.y < 0.0) ? 2 : 3;
    visibleFace.z = (direction.z < 0.0) ? 4 : 5;
	//�������Բ���Ȩֵ
	vec3 weight = direction * direction;
	//��������ײ 
	float worldVoxelScale = 2.0f * voxelScale;
    float dst = worldVoxelScale;
    vec3 startPosition = position + direction * dst;

	//���ӶȺ��ڱ�ֵ
    float visibility = 0.0f;
    float k = exp2(0.7f);

    //�Ƿ��볡���ཻ
    float enter = 0.0; float leave = 0.0;
    if(!IsIntersectWithWorldAABB(position, direction, enter, leave))
    {
        visibility = 1.0f;
    }
    
    while(visibility < 1.0f && dst <= maxTracingDistance)
    {
        vec3 conePosition = startPosition + direction * dst;
        float diameter = 2.0f * tan(aperture) * dst;
        float mipLevel = log2(diameter / worldVoxelScale);
        //��������ת��Ϊ3d��������
        vec3 coord = WorldToVoxel(conePosition);
        //�������Բ���
        vec4 anisoSample = AnistropicSample(coord, weight, visibleFace, mipLevel); 
        //vec4 anisoSample = texture(voxelRadiance, coord); 
		//step
		//visibility += (1.0f - visibility)*anisoSample.a * k;
//		if(anisoSample.a > 0){
//			visibility += (1.0f - visibility) * k;
//		}
        visibility += (1.0f - visibility) * anisoSample.a * k;
        dst += diameter * 0.5f;
    }

    return 1.0f - visibility;
}