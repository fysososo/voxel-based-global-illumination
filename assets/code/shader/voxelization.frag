#version 450 core

in GeometryOut
{
    vec3 wsPosition;
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    flat vec4 triangleAABB;
} In;

out vec4 fragColor;

layout(binding = 0, r32ui)  uniform volatile coherent uimage3D texture_albedo;
layout(binding = 1, r32ui)  uniform volatile coherent uimage3D texture_normal;
layout(binding = 2, r32ui)  uniform volatile coherent uimage3D texture_emission;

uniform bool hasAlbedoMap;
uniform bool hasNormalMap;
uniform bool hasEmissionMap;

layout(binding = 0) uniform sampler2D AlbedoMap;
layout(binding = 1) uniform sampler2D EmissionMap;
layout(binding = 2) uniform sampler2D NormalMap;

uniform vec3 albedo;
uniform vec3 emission;

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
	if( In.position.x < In.triangleAABB.x || In.position.y < In.triangleAABB.y || 
		In.position.x > In.triangleAABB.z || In.position.y > In.triangleAABB.w )
	{
		discard;
	}

	ivec3 iposition = ivec3(In.wsPosition);

	//体素化albedo
	vec4 albedoData;
	if(hasAlbedoMap){
		albedoData = texture(AlbedoMap, In.texCoord);
	}
	else{
		albedoData = vec4(albedo, 1.0f);
	}
	imageAtomicRGBA8Avg(texture_albedo, iposition, albedoData);

	//体素化法线
	vec3 normalData = EncodeNormal(normalize(In.normal));
	imageAtomicRGBA8Avg(texture_normal, iposition, vec4(normalData,1.0f));

	//体素化自发光
	vec4 emissionData;
	if(hasEmissionMap){
		emissionData = vec4(texture(EmissionMap, In.texCoord).xyz + emission, 1.0f);
	}
	else{
		emissionData = vec4(emission, 1.0f);
	}
	imageAtomicRGBA8Avg(texture_emission, iposition, vec4(emissionData.xyz,1.0f));
}