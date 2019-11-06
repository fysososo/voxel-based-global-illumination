#version 430 core
#extension GL_ARB_explicit_uniform_location : enable
in vec2 TexCoord;
out vec4 fragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gRoughness;
uniform sampler2D gMetalness;
uniform sampler2D gAbledo;

//±‡–¥BSDF‰÷»æƒ£–Õ
//°≠°≠

void main(){
	vec3 pos = texture(gPosition, TexCoord).xyz;
	vec3 albedo = texture(gAbledo, TexCoord).xyz;
	float roughness = texture(gRoughness, TexCoord).r;
	float metalness = texture(gMetalness, TexCoord).r;

	vec3 L = normalize(lightPos - pos);
	vec3 V = normalize(viewPos - pos);
	vec3 N = texture(gNormal, TexCoord).xyz;
	vec3 H = normalize(L+V);

	float diff = max(dot(L, N), 0.0);
	float spec = pow(max(dot(N, H), 0.0), 32.0);

    vec3 specularColor = vec3(0.2) * spec;
	vec3 diffuseColor = diff*albedo;

	fragColor = vec4(specularColor+diffuseColor,1.0f);
}