#version 450 core
#extension GL_ARB_explicit_uniform_location : enable
in vec3 normal;
out vec4 FragColor;

uniform sampler2D texture_diffuse;

void main()
{    
	vec3 norm = normalize(normal);
	FragColor = vec4(1.0f,0.0f,1.0f,1.0f)*max(dot(norm, vec3(1.0f,1.0f,1.0f)), 0);
}