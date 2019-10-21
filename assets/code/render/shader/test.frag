#version 330 core
#extension GL_ARB_explicit_uniform_location : enable

out vec4 FragColor;
in float outColor;
in vec2 TexCoord;
layout (location = 0) uniform float blueValue;
layout (location = 1) uniform sampler2D ourTexture;

void main()
{
	FragColor =  mix(texture(ourTexture, TexCoord), vec4(1.0f * outColor, 0.5 * outColor, blueValue * outColor, 1.0f), 0.4);
}