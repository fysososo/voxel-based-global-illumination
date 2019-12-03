#version 450 core
layout (location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

uniform mat4 model;

out Vertex
{
	vec2 texCoord;
	//vec3 normal;
};

void main(){
	gl_Position = model*vec4(vertexPosition, 1.0f);
	texCoord = vertexTexCoord;
}