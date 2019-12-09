#version 450 core
layout (location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location=3) in vec3 vertexTangent;
layout(location=4) in vec3 vertexBiTangent;

uniform mat4 model;

out Vertex
{
	vec2 texCoord;
	mat3 TBN;
};

void main(){
	gl_Position = model*vec4(vertexPosition, 1.0f);
	texCoord = vertexTexCoord;
	vec3 T = normalize(vec3(model * vec4(vertexTangent,   0.0)));
	vec3 B = normalize(vec3(model * vec4(vertexBiTangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(vertexNormal,    0.0)));
	TBN = mat3(T, B, N);
}