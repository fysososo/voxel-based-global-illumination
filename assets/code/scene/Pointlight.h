#pragma once
#include<stdafx.h>
class Pointlight
{
public:
	glm::vec3 position;
	glm::vec3 color;
	float intensity;

	Pointlight(glm::vec3 pos, glm::vec3 color, float intensity) :position(pos), color(color), intensity(intensity) {}
	~Pointlight() {};
};

