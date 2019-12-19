#version 450 core

in vec3 FragPos;
in vec2 TexCoord;
in vec3 ClipPos;
in vec4 BoundingBox;
in mat3 TBN;

out vec4 fragColor;

layout(binding = 0, r32ui)  uniform volatile coherent uimage3D texture_albedo;
layout(binding = 1, r32ui)  uniform volatile coherent uimage3D texture_normal;
layout(binding = 2, r32ui)  uniform volatile coherent uimage3D texture_emission;
layout(binding = 3, r32ui)  uniform volatile coherent uimage3D texture_roughness;
layout(binding = 4, r32ui)  uniform volatile coherent uimage3D texture_metalness;

uniform bool hasAlbedoMap;
uniform sampler2D AlbedoMap;
uniform vec3 albedo;

uniform bool hasNormalMap;
uniform sampler2D NormalMap;

uniform bool hasEmissionMap;
uniform sampler2D EmissionMap;
uniform vec3 emission;

uniform bool hasRoughnessMap;
uniform sampler2D RoughnessMap;
uniform float roughness;

uniform bool hasMetalnessMap;
uniform sampler2D MetalnessMap;
uniform float metalness;

uniform float voxelSize;
uniform vec3 boxMin;

//32uint-->vec4
vec4 convRGBA8ToVec4(uint val)
{
	return vec4(float((val & 0x000000FF)), 
	float((val & 0x0000FF00) >> 8U), 
	float((val & 0x00FF0000) >> 16U), 
	float((val & 0xFF000000) >> 24U));
}

//vec4-->32uint
uint convVec4ToRGBA8(vec4 val)
{
	return (uint(val.w) & 0x000000FF) << 24U | 
	(uint(val.z) & 0x000000FF) << 16U | 
	(uint(val.y) & 0x000000FF) << 8U | 
	(uint(val.x) & 0x000000FF);
}

void imageAtomicRGBA8Avg(layout(r32ui) volatile coherent uimage3D grid, ivec3 coords, vec4 value)
{
    value.rgb *= 255.0;                 // optimize following calculations
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;
    uint numIterations = 0;

    while((curStoredVal = imageAtomicCompSwap(grid, coords, prevStoredVal, newVal)) 
            != prevStoredVal
            && numIterations < 255)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.rgb = (rval.rgb * rval.a); // Denormalize
        vec4 curValF = rval + value;    // Add
        curValF.rgb /= curValF.a;       // Renormalize
        newVal = convVec4ToRGBA8(curValF);

        ++numIterations;
    }
}

vec3 EncodeNormal(vec3 normal)
{
    return normal * 0.5f + vec3(0.5f);
}

vec3 DecodeNormal(vec3 normal)
{
    return normal * 2.0f - vec3(1.0f);
}

void main()
{   
	//剔除因保守光栅化产生的多余体素
	if( ClipPos.x < BoundingBox.x || ClipPos.y < BoundingBox.y || 
		ClipPos.x > BoundingBox.z || ClipPos.y > BoundingBox.w )
	{
		discard;
	}

	int x = int((FragPos.x - boxMin.x)/voxelSize);
	int y = int((FragPos.y - boxMin.y)/voxelSize);
	int z = int((FragPos.z - boxMin.z)/voxelSize);

	//体素化albedo
	vec4 albedoData;
	if(hasAlbedoMap){
		albedoData = vec4(vec3(
		pow(texture(AlbedoMap, TexCoord).x, 2.2),
		pow(texture(AlbedoMap, TexCoord).y, 2.2),
		pow(texture(AlbedoMap, TexCoord).z, 2.2)) + albedo, 1.0f);
	}
	else{
		albedoData = vec4(albedo, 1.0f);
	}
	imageAtomicRGBA8Avg(texture_albedo, ivec3(x,y,z), albedoData);

	//体素化法线
	vec3 normalData;
	if(hasNormalMap){
		normalData = EncodeNormal(normalize(TBN*DecodeNormal(texture(NormalMap, TexCoord).rgb)));
	}
	else{
		normalData = EncodeNormal(normalize(TBN[2]));
	}
	imageAtomicRGBA8Avg(texture_normal, ivec3(x,y,z), vec4(normalData,1.0f));

	//体素化自发光
	vec4 emissionData;
	if(hasEmissionMap){
		emissionData = vec4(texture(EmissionMap, TexCoord).xyz + emission, 1.0f);
	}
	else{
		emissionData = vec4(emission, 1.0f);
	}
	imageAtomicRGBA8Avg(texture_emission, ivec3(x,y,z), vec4(emission.xyz,1.0f));

	//体素化粗超度
	vec4 roughnessData;
	if(hasRoughnessMap){
		roughnessData = texture(RoughnessMap, TexCoord) + vec4(roughness);
	}else{
		roughnessData = vec4(roughness);
	}
	imageAtomicRGBA8Avg(texture_roughness, ivec3(x,y,z), vec4(vec3(roughnessData.r),1.0f));
	
	//体素化金属度vec4 roughnessData;
	vec4 metalnessData;
	if(hasMetalnessMap){
		metalnessData = texture(MetalnessMap, TexCoord) + vec4(metalness);
	}else{
		roughnessData = vec4(roughness);
	}
	imageAtomicRGBA8Avg(texture_metalness, ivec3(x,y,z), vec4(vec3(metalnessData.r),1.0f));
	
}