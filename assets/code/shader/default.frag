#version 450 core
#extension GL_ARB_explicit_uniform_location : enable

out vec4 FragColor;

uniform sampler2D texture_diffuse;

void main()
{    
	FragColor = vec4(1.0f,1.0f,1.0f,1.0f);
}