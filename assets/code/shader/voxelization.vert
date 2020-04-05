#version 450 core
layout (location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;
layout(location=3) in vec3 vertexTangent;
layout(location=4) in vec3 vertexBiTangent;

uniform struct Matrices
{
    mat4 model;
    mat4 normal;
} matrices;

out Vertex
{
	vec2 texCoord;
	vec3 normal;
};

void main(){
	gl_Position = matrices.model*vec4(vertexPosition, 1.0f);
	normal = (matrices.normal * vec4(vertexNormal, 0.0f)).xyz;
	texCoord = vertexTexCoord;
}