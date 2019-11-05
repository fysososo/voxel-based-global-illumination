#version 430 core
in vec2 TexCoord;
out vec4 fragColor;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gRoughness;
layout (location = 4) out vec4 gMetalness;

uniform sampler2D NormalMap;
uniform sampler2D AlbedoMap;
uniform sampler2D RoughnessMap;
uniform sampler2D MetalnessMap;

void main(){
	//将贴图中的信息处理后存入各个GBuffer
	//……
	lowp vec4 nor1 =  texture(NormalMap, TexCoord);
	fragColor = nor1;
}