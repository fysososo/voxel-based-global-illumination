#version 450 core

in vec3 FragPos;
in vec2 TexCoord;
in vec3 ClipPos;
in vec4 BoundingBox;

out vec4 fragColor;

layout(binding = 0, r32ui)  uniform volatile coherent uimage3D texture_albedo;

uniform bool hasAlbedoMap;
uniform sampler2D AlbedoMap;
uniform vec3 albedo;

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
	value *= 255.0;
	uint newVal = convVec4ToRGBA8(value);
	uint prevStoredVal = 0;
	uint curStoredVal;
	uint numIterations = 0;

	//若未存值，则存值，进入不了循环
	//若已有值，则不存值，并进入循环；两个值在循环中按照alpha值混合后，存值，离开循环
	while((curStoredVal = imageAtomicCompSwap(grid, coords, prevStoredVal, newVal)) 
			!= prevStoredVal
			&& numIterations < 255
			)
	{
		prevStoredVal = curStoredVal;
		vec4 rval = convRGBA8ToVec4(curStoredVal);
		rval.rgb = (rval.rgb * rval.a); // Denormalize
		vec4 curValF = rval + value;    // Add
		curValF.rgb /= curValF.a;       // Renormalize
		if(curValF.a > 255){
			curValF.a = 255;
		}
		newVal = convVec4ToRGBA8(curValF);
		++numIterations;

	}
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
	vec4 albedoData;
	if(hasAlbedoMap){
		albedoData = vec4(texture(AlbedoMap, TexCoord).xyz, 1.0f);
	}
	else{
		albedoData = vec4(albedo, 1.0f);
	}

	imageAtomicRGBA8Avg(texture_albedo, ivec3(x,y,z), albedoData);
}