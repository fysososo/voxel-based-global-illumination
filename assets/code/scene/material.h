#pragma once
#include <stdafx.h>


class Material
{
public:
	string name;//材质名
	glm::vec4 Ka;//颜色光照albedo
	glm::vec4 Kd;//漫反射diffuse
	glm::vec4 Ks;//镜面反射specular
			
	vector<shared_ptr<Texture>> diffuseMaps;//漫反射贴图
	vector<shared_ptr<Texture>> specularMaps;//镜面反射贴图
	vector<shared_ptr<Texture>> normalMaps;//切线空间的法线贴图

	~Material();
};

