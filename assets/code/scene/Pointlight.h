#pragma once
#include<stdafx.h>
class Pointlight
{
public:
	//点光源衰减参数
	//公式：falloff = 1 / (1 + constant + (linear * d) + (quadratic * d * d)),其中d为光源与物体的距离
	float constant;
	float linear;
	float quadratic;

	//点光源位置
	glm::vec3 position;
	//点光源颜色
	glm::vec3 color;

	//x为环境光强度分量， y为漫反射光环境分量，z为镜面反射环境变量
	glm::vec3 intensity;
	
	Pointlight(glm::vec3 pos, glm::vec3 color, glm::vec3 intensity = glm::vec3(1.0f),
		glm::vec3 ambient = glm::vec3(0.0f), glm::vec3 diffuse = glm::vec3(1.0f), glm::vec3 specular = glm::vec3(1.0f),
		float constant = 1.0f, float linear = 0.2f, float quadratic = 0.08f
	) 
		:position(pos), 
		color(color), 
		intensity(intensity),
		ambient(ambient),
		diffuse(diffuse),
		specular(specular),
		constant(constant),
		linear(linear),
		quadratic(quadratic)
	{}

	glm::vec3 Ambient();
	glm::vec3 Diffuse();
	glm::vec3 Specular();

	~Pointlight() {};

private:
	//环境光
	glm::vec3 ambient;
	//漫反射光
	glm::vec3 diffuse;
	//镜面反射
	glm::vec3 specular;
};

