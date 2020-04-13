#version 450 core

out vec4 albedo;
layout(binding = 0) uniform sampler3D voxelImage;

uniform int dimension;
uniform int mipLevel;

void main()
{
		vec3 position = vec3
	(
		gl_VertexID % dimension,
		(gl_VertexID / dimension) % dimension,
		gl_VertexID / (dimension * dimension)
	);

	ivec3 texPos = ivec3(position);
	albedo = texelFetch(voxelImage, texPos, mipLevel);

	gl_Position = vec4(position, 1.0f);
}