#version 430 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
layout(location=3) in vec3 aTangent;
layout(location=4) in vec3 aBiTangent;

out mat3 TBN;
out vec2 TexCoord;
out vec3 Position;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main(){
	TexCoord = aTexCoord;
	vec3 T = normalize((model * vec4(aTangent,   0.0)).xyz);
	vec3 B = normalize((model * vec4(aBiTangent, 0.0)).xyz);
	vec3 N = normalize((model * vec4(aNormal,    0.0)).xyz);
	TBN = mat3(T, B, N);
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	Position = (model * vec4(aPos,1.0f)).xyz;
}