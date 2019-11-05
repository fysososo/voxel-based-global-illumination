#version 430 core
#extension GL_ARB_explicit_uniform_location : enable
in vec2 TexCoord;
out vec4 fragColor;
uniform sampler3D voxelNormal;
uniform sampler3D voxelRadiance;
uniform sampler3D voxelIOR;

void main(){
	fragColor = vec4(1.0f,0.0f,1.0f,1.0f);
}