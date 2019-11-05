#include <stdafx.h>
#include "material.h"

void Material::loadMaterial(string path)
{
	modelPath = path.substr(0, path.find_last_of('/'));
	fstream matFile(path);
	if (!matFile.is_open()) {
		cout << "Material::loadMaterial::打开材质文件失败！将使用默认材质！" << endl;
		return;
	}
	matName = path.substr(path.find_last_of('/') + 1, path.find(".") - path.find_last_of('/') - 1);
	while (matFile && !matFile.eof()) {
		string property;
		matFile >> property;
		if (property == "roughness") {
			matFile >> roughness;
		}
		else if (property == "IOR") {
			matFile >> IOR;
		}
		else if (property == "metalness") {
			matFile >> metalness;
		}
		else if (property == "F0") {
			matFile >> F0.x >> F0.y >> F0.z;
		}
		else if (property == "albedo") {
			matFile >> albedo.x >> albedo.y >> albedo.z;
		}
		else if (property == "NormalMap") {
			int hasNormalMap;
			matFile >> hasNormalMap;
			if (hasNormalMap) {
				loadTexture(en_TEXTURE_NORMAL);
			}
		}
		else if (property == "RoughnessMap") {
			int hasRoughnessMap;
			matFile >> hasRoughnessMap;
			if (hasRoughnessMap) {
				loadTexture(en_TEXTURE_ROUGHNESS);
			}
		}
		else if (property == "MetalnessMap") {
			int hasMetalnessMap;
			matFile >> hasMetalnessMap;
			if (hasMetalnessMap) {
				loadTexture(en_TEXTURE_METANESS);
			}
		}
		else if (property == "AlbedoMap") {
			int hasAlbedoMap;
			matFile >> hasAlbedoMap;
			if (hasAlbedoMap) {
				loadTexture(en_TEXTURE_ALBEDO);
			}
		}
		else {
			cout << "Material::loadMaterial::错误的材质文件属性" + property + "！请检查pbr文件！";
		}
	}
	matFile.close();
}

Material::Material()
{
	modelPath = "";
	matName = "";
	roughness = 0;
	metalness = 0;
	F0 = glm::vec3(0, 0, 0);
	albedo = glm::vec3(0, 0, 0);
	IOR = 1;
	metalnessMap = NULL;
	roughnessMap = NULL;
	albedoMap = NULL;
	normalMap = NULL;
}

Material::~Material()
{
}

void Material::loadTexture(en_textureType type)
{
	string fileName;
	switch (type)
	{
	case Material::en_TEXTURE_NORMAL:
		fileName = matName + "_NORMAL.jpg";
		normalMap = bindTexture(type, modelPath + "/textures/" + fileName);
		break;
	case Material::en_TEXTURE_ROUGHNESS:
		fileName = matName + "_ROUGHNESS.jpg";
		roughnessMap = bindTexture(type, modelPath + "/textures/" + fileName);
		break;
	case Material::en_TEXTURE_METANESS:
		fileName = matName + "_METALNESS.jpg";
		metalnessMap = bindTexture(type, modelPath + "/textures/" + fileName);
		break;
	case Material::en_TEXTURE_ALBEDO:
		fileName = matName + "_ALBEDO.jpg";
		normalMap = bindTexture(type, modelPath + "/textures/" + fileName);
		break;
	default:
		break;
	}
}

unsigned int Material::bindTexture(en_textureType type, string path)
{
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// 为当前绑定的纹理对象设置环绕、过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 加载并生成纹理
	switch (type)
	{
	case en_TEXTURE_ALBEDO:
	case en_TEXTURE_NORMAL:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		break;
	case en_TEXTURE_METANESS:
	case en_TEXTURE_ROUGHNESS:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
		break;
	}
	
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	return texture;
}
