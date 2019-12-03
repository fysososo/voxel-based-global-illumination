#version 450 core

out vec4 albedo;
layout(binding = 0, rgba8) uniform readonly image3D texture_albedo;

uniform uint dimension;


void main()
{
		vec3 position = vec3
	(
		gl_VertexID % dimension,
		(gl_VertexID / dimension) % dimension,
		gl_VertexID / (dimension * dimension)
	);

	ivec3 texPos = ivec3(position);
	albedo = imageLoad(texture_albedo, texPos);


	gl_Position = vec4(position, 1.0f);
}