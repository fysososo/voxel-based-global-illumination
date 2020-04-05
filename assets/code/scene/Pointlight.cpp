#include "stdafx.h"
#include "Pointlight.h"

glm::vec3 Pointlight::Ambient()
{
	return ambient*intensity[0];
}

glm::vec3 Pointlight::Diffuse()
{
	return diffuse*intensity[1];
}

glm::vec3 Pointlight::Specular()
{
	return specular*intensity[2];
}
