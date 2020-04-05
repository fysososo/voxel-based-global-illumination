#version 430

in vec2 TexCoord;
layout(location = 0) out vec4 fragColor;

//������
const float PI = 3.14159265f;
const float HALF_PI = 1.57079f;
const float EPSILON = 1e-30;

//�ƹ�
/*���Դ˥������*/
struct Attenuation
{
    float constant;
    float linear;
    float quadratic;
};
/*�ƹ�ṹ��*/
struct Light {
    Attenuation attenuation;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 position;
    vec3 direction;
};
/*�ƹ���Ϣ*/
const uint MAX_POINT_LIGHTS = 6;
uniform Light pointLight[MAX_POINT_LIGHTS];
uniform int lightCount;

//����
uniform vec3 viewPos;
uniform float coneShadowTolerance = 0.1f;
uniform float coneShadowAperture = 0.03f;
uniform float bounceStrength = 1.0f;
uniform float aoFalloff = 800.0f;
uniform float aoAlpha = 0.01f;
uniform float samplingFactor = 1.0f;
uniform float maxTracingDistanceGlobal = 0.95f;
const vec3 diffuseConeDirections[] =
{
    vec3(0.0f, 1.0f, 0.0f),
    vec3(0.0f, 0.5f, 0.866025f),
    vec3(0.823639f, 0.5f, 0.267617f),
    vec3(0.509037f, 0.5f, -0.7006629f),
    vec3(-0.50937f, 0.5f, -0.7006629f),
    vec3(-0.823639f, 0.5f, 0.267617f)
};

const float diffuseConeWeights[] =
{
    PI / 4.0f,
    3.0f * PI / 20.0f,
    3.0f * PI / 20.0f,
    3.0f * PI / 20.0f,
    3.0f * PI / 20.0f,
    3.0f * PI / 20.0f,
};

uniform int showMode = 0;

//�����û���
layout(binding = 0, rgba16f) uniform volatile coherent image2D gDebug;

//�ӳ���Ⱦ���ν׶ν��
layout(binding = 1) uniform sampler2D gPosition;
layout(binding = 2) uniform sampler2D gNormal;
layout(binding = 3) uniform sampler2D gRoughness;
layout(binding = 4) uniform sampler2D gMetalness;
layout(binding = 5) uniform sampler2D gAbledo;
layout(binding = 6) uniform sampler2D gEmission;
layout(binding = 7) uniform sampler2D gSpecular;

//����3d��������
layout(binding = 8) uniform sampler3D voxelRadiance;
layout(binding = 9) uniform sampler3D voxelTexMipmap[6];


//��������
uniform float voxelScale;
uniform float voxelSize;
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
vec3 BRDF_t(Light light, vec3 N, vec3 X, vec3 ka, vec4 ks);
vec3 Ambient(Light light, vec3 albedo);
vec3 CalculatePoint(Light light, vec3 normal, vec3 position, vec3 albedo, vec4 specular);
#define PI 3.1415926f

//�Ƿ��볡���ཻ
bool IsIntersectWithWorldAABB(vec3 o, vec3 d, out float leave, out float enter);

//��������仯Ϊ��������
vec3 WorldToVoxel(vec3 position);

//�������Բ���
vec4 AnistropicSample(vec3 coord, vec3 weight, uvec3 face, float lod);

//�ӽ�׶׷�ٲ���
vec4 TraceCone(vec3 position, vec3 normal, vec3 direction, float aperture, bool traceOcclusion);

//�ӽ�׶������Ӱ
float TraceShadowCone(vec3 position, vec3 direction, float aperture, float maxTracingDistance);

//�����ӹ���
vec4 CalculateIndirectLighting(vec3 position, vec3 normal, vec3 albedo, vec4 specular, bool ambientOcclusion);

//����ֱ�ӹ���
vec3 CalculateDirectLighting(vec3 position, vec3 normal, vec3 albedo, vec4 specular);

vec3 EncodeNormal(vec3 normal)
{
    return normal * 0.5f + vec3(0.5f);
}

vec3 DecodeNormal(vec3 normal)
{
    return normal * 2.0f - vec3(1.0f);
}


void main(){
	vec3 pos = texture(gPosition, TexCoord).xyz;
	vec4 baseColor = texture(gAbledo, TexCoord);
    vec3 albedo = pow(baseColor.xyz, vec3(2.2f));
    // xyz = specular, w = shininess
    vec4 specular = texture(gSpecular, TexCoord);
	vec4 roughness = texture(gRoughness, TexCoord);
    vec4 emission = texture(gEmission, TexCoord);
	float metalness = roughness.g;
	float F0 = roughness.b;
	
	vec3 V = normalize(viewPos - pos);
	vec3 N = normalize(texture(gNormal, TexCoord).xyz);
    //����ֱ�ӹ���
	vec4 directLight = vec4(0.0f);
	directLight.rgb = CalculateDirectLighting(pos, N, albedo, specular);
    vec4 directLightWithoutShadow = directLight;
    //�����ӹ���
	vec4 indirectLighting = CalculateIndirectLighting(pos,N,albedo.rgb,specular, true);
	indirectLighting.rgb = pow(indirectLighting.rgb, vec3(2.2f));
    //��Ϻ�Ľ��
    vec3 compositeLighting;
    if(showMode == 0){
        //��ӹ�+ֱ�ӹ�+��Ӱ
	    compositeLighting = (directLight.rgb + indirectLighting.rgb)*indirectLighting.a;
    }
    else if(showMode == 1){
        //ֱ�ӹ�+��Ӱ
	    compositeLighting = directLight.rgb;
    }
    else if(showMode == 2){
        //��ӹ�+ֱ�ӹ�
	    compositeLighting = (directLightWithoutShadow.rgb + indirectLighting.rgb);
    }
    else if(showMode == 3){
        //��ӹ�
        compositeLighting = indirectLighting.rgb;
    }
    else{
        //ֱ�ӹ�
        compositeLighting = directLightWithoutShadow.rgb;
    }
    //����Է���
    compositeLighting.rgb += emission.rgb;
	compositeLighting = compositeLighting / (compositeLighting + 1.0f);
	//gammaУ��
    const float gamma = 2.2;
    //ת����gamma�ռ�
     compositeLighting = pow(compositeLighting, vec3(1.0 / gamma));

    fragColor = vec4(compositeLighting, 1.0f);
}

vec3 CalculateDirectLighting(vec3 position, vec3 normal, vec3 albedo, vec4 specular){
    // calculate directional lighting
    vec3 directLighting = vec3(0.0f);

    // calculate lighting for point lights
    for(int i = 0; i < lightCount; ++i)
    {
        directLighting += CalculatePoint(pointLight[i], normal, position, 
                                   albedo, specular);
        directLighting += Ambient(pointLight[i], albedo);
    }

    return directLighting;

}

vec3 CalculatePoint(Light light, vec3 normal, vec3 position, vec3 albedo, vec4 specular)
{
    light.direction = light.position - position;
    float d = length(light.direction);
    light.direction = normalize(light.direction);
    float falloff = 1.0f / (light.attenuation.constant + light.attenuation.linear * d
                    + light.attenuation.quadratic * d * d + 1.0f);

    if(falloff <= 0.0f) return vec3(0.0f);

    float visibility = 1.0f;
    visibility = max(0.0f, TraceShadowCone(position, light.direction, coneShadowAperture, d));


    if(visibility <= 0.0f) return vec3(0.0f);  

    return BRDF_t(light, normal, position, albedo, specular) * falloff * visibility;
}

vec3 Ambient(Light light, vec3 albedo)
{
    return max(albedo * light.ambient, 0.0f);
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

vec3 BRDF(vec3 albedo, vec3 N,vec3 L,vec3 H, vec3 V, float roughness, float metalness, vec3 F, float KD){
	vec3 diff = albedo/PI;
	vec3 spec = f_Specular(N, L, H, V, roughness, metalness, F);
	return (KD*diff+(1-KD)*(1-metalness)*spec);
}

vec3 BRDF_t(Light light, vec3 N, vec3 X, vec3 ka, vec4 ks)
{
    // common variables
    vec3 L = light.direction;
    vec3 V = normalize(viewPos - X);
    vec3 H = normalize(V + L);
    // compute dot procuts
    float dotNL = max(dot(N, L), 0.0f);
    float dotNH = max(dot(N, H), 0.0f);
    float dotLH = max(dot(L, H), 0.0f);
    // decode specular power
    float spec = exp2(11.0f * ks.a + 1.0f);
    // emulate fresnel effect
    vec3 fresnel = ks.rgb + (1.0f - ks.rgb) * pow(1.0f - dotLH, 5.0f);
    // specular factor
    float blinnPhong = pow(dotNH, spec);
    // energy conservation, aprox normalization factor
    blinnPhong *= spec * 0.0397f + 0.3183f;
    // specular term
    vec3 specular = ks.rgb * light.specular * blinnPhong * fresnel;
    // diffuse term
    vec3 diffuse = ka.rgb * light.diffuse;
    // return composition
    return (diffuse + specular) * dotNL;
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

vec3 WorldToVoxel(vec3 position){
	vec3 voxelPos = position - worldMinPoint;
    return voxelPos * voxelScale;
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

float TraceShadowCone(vec3 position, vec3 direction, float aperture, float maxTracingDistance){
	bool hardShadows = false;

    if(coneShadowTolerance == 1.0f) { hardShadows = true; }

    //�ɼ���
    uvec3 visibleFace;
    visibleFace.x = (direction.x < 0.0) ? 0 : 1;
    visibleFace.y = (direction.y < 0.0) ? 2 : 3;
    visibleFace.z = (direction.z < 0.0) ? 4 : 5;
    //����ռ��µ����ش�С
    float voxelWorldSize = 2.0 /  (voxelScale * volumeDimension);
    //�������Ȩ��
    vec3 weight = direction * direction;
    //��ֹ����ײ
    float dst = voxelWorldSize;
    vec3 startPosition = position + direction * dst;
    //���mipmap�㼶
    float mipMaxLevel = log2(volumeDimension) - 1.0f;
    // ���ս��
    float visibility = 0.0f;
    //�ۼ�ʱ�Ĳ���
    float k = exp2(7.0f * coneShadowTolerance);
    //�׷�پ���
    float maxDistance = maxTracingDistance;
    //�Ƿ����
    float enter = 0.0; float leave = 0.0;

    if(!IsIntersectWithWorldAABB(position, direction, enter, leave))
    {
        visibility = 1.0f;
    }
    
    while(visibility < 1.0f && dst <= maxDistance)
    {
        vec3 conePosition = startPosition + direction * dst;
//        if(conePosition.x > worldMaxPoint.x || conePosition.y > worldMaxPoint.y || conePosition.z > worldMaxPoint.z
//            || conePosition.x < worldMinPoint.x || conePosition.y < worldMinPoint.y || conePosition.z < worldMinPoint.z){
//            break;
//        }
        float diameter = 2.0f * aperture * dst;
        float mipLevel = log2(diameter / voxelWorldSize);
        //ת������������ռ�
        vec3 coord = WorldToVoxel(conePosition);
        //�������Բ���
        vec4 anisoSample = AnistropicSample(coord, weight, visibleFace, mipLevel);

        //Ӳ��Ӱ
        if(hardShadows && anisoSample.a > EPSILON) { return 0.0f; }  
        //�ۼ�
        visibility += (1.0f - visibility) * anisoSample.a * k;
        //step
        dst += diameter * samplingFactor;
    }
    return 1.0f - visibility;
}

int temp;
vec4 TraceCone(vec3 position, vec3 normal, vec3 direction, float aperture, bool traceOcclusion)
{
    uvec3 visibleFace;
    visibleFace.x = (direction.x < 0.0) ? 0 : 1;
    visibleFace.y = (direction.y < 0.0) ? 2 : 3;
    visibleFace.z = (direction.z < 0.0) ? 4 : 5;
    traceOcclusion = traceOcclusion && aoAlpha < 1.0f;
    //����ռ��µ����ش�С
    float voxelWorldSize = 2.0 / (voxelScale * volumeDimension);
    //�������Ȩ��
    vec3 weight = direction * direction;
    // ��ǰ�ƶ���������ײ
    float dst = voxelWorldSize;
    vec3 startPosition = position + normal * dst;
    //�������
    vec4 coneSample = vec4(0.0f);
    float occlusion = 0.0f;
    float maxDistance = maxTracingDistanceGlobal * (1.0f / voxelScale);
	float falloff = 0.5f * aoFalloff * voxelScale;
    //������
    float enter = 0.0; float leave = 0.0;

    if(!IsIntersectWithWorldAABB(position, direction, enter, leave))
    {
        coneSample.a = 1.0f;
    }
    int isFirst = 0;
    while(coneSample.a < 1.0f && dst <= maxDistance)
    {
        vec3 conePosition = startPosition + direction * dst;
//        if(conePosition.x > worldMaxPoint.x || conePosition.y > worldMaxPoint.y || conePosition.z > worldMaxPoint.z
//            || conePosition.x < worldMinPoint.x || conePosition.y < worldMinPoint.y || conePosition.z < worldMinPoint.z){
//            break;
//        }
        //����mipmap
        float diameter = 2.0f * aperture * dst;
        float mipLevel = log2(diameter / voxelWorldSize);
        //ת������������ռ�
        vec3 coord = WorldToVoxel(conePosition);
        //�������Բ���
        vec4 anisoSample = AnistropicSample(coord, weight, visibleFace, mipLevel);
        //����
        coneSample += (1.0f - coneSample.a) * anisoSample;
        //ȫ������
        if(traceOcclusion && occlusion < 1.0)
        {
            occlusion += ((1.0f - occlusion) * anisoSample.a) / (1.0f + falloff * diameter);
        }
        //step
        dst += diameter * samplingFactor;
    }
    
    return vec4(coneSample.rgb, occlusion);
}

vec4 CalculateIndirectLighting(vec3 position, vec3 normal, vec3 albedo, vec4 specular, bool ambientOcclusion)
{
    vec4 specularTrace = vec4(0.0f);
    vec4 diffuseTrace = vec4(0.0f);
    vec3 coneDirection = vec3(0.0f);

    //specular��Ϊ0
    if(any(greaterThan(specular.rgb, specularTrace.rgb)))
    {
        vec3 viewDirection = normalize(viewPos - position);
        vec3 coneDirection = reflect(-viewDirection, normal);
        coneDirection = normalize(coneDirection);
        // specular cone setup, minimum of 1 grad, fewer can severly slow down performance
        float aperture = clamp(tan(HALF_PI * (1.0f - specular.a)), 0.0174533f, PI);
        specularTrace = TraceCone(position, normal, coneDirection, aperture, ambientOcclusion);
        specularTrace.rgb *= specular.rgb;
    }

    //��ǰalbedo������
    if(any(greaterThan(albedo, diffuseTrace.rgb)))
    {
        //�������������
        const float aperture = 0.57735f;
        //const float aperture = 0.01f;
        vec3 guide = vec3(0.0f, 1.0f, 0.0f);

        if (abs(dot(normal,guide)) == 1.0f)
        {
            guide = vec3(0.0f, 0.0f, 1.0f);
        }

        // �������ߺ͸�����
        vec3 right = normalize(guide - dot(normal, guide) * normal);
        vec3 up = cross(right, normal);

        for(int i = 0; i < 6; i++)
        {
            coneDirection = normal;
            coneDirection += diffuseConeDirections[i].x * right + diffuseConeDirections[i].z * up;
            coneDirection = normalize(coneDirection);
            //����
            diffuseTrace += TraceCone(position, normal, coneDirection, aperture, ambientOcclusion) * diffuseConeWeights[i];
        }
        diffuseTrace.rgb *= albedo;
    }
    vec3 result = bounceStrength * (diffuseTrace.rgb + specularTrace.rgb);
    //vec3 result = bounceStrength * diffuseTrace.rgb;
    //vec3 result = bounceStrength * specularTrace.rgb;

    return vec4(result, ambientOcclusion ? clamp(1.0f - diffuseTrace.a + aoAlpha, 0.0f, 1.0f) : 1.0f);
}