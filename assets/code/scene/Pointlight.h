#pragma once
#include<stdafx.h>
class Pointlight
{
public:
	//���Դ˥������
	//��ʽ��falloff = 1 / (1 + constant + (linear * d) + (quadratic * d * d)),����dΪ��Դ������ľ���
	float constant;
	float linear;
	float quadratic;

	//���Դλ��
	glm::vec3 position;
	//���Դ��ɫ
	glm::vec3 color;

	//xΪ������ǿ�ȷ����� yΪ������⻷��������zΪ���淴�价������
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
	//������
	glm::vec3 ambient;
	//�������
	glm::vec3 diffuse;
	//���淴��
	glm::vec3 specular;
};

