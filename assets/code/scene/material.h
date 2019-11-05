#pragma once
#include <stdafx.h>
class Material
{

public:

	enum en_textureType {
		en_TEXTURE_NORMAL = 0,
		en_TEXTURE_ROUGHNESS,
		en_TEXTURE_METANESS,
		en_TEXTURE_ALBEDO
	};

	string matName;

	float roughness;
	float IOR;
	float metalness;
	glm::vec3 F0;
	glm::vec3 albedo;
	
	unsigned int normalMap;
	unsigned int roughnessMap;
	unsigned int metalnessMap;
	unsigned int albedoMap;


	void loadMaterial(string path);
	Material();
	~Material();

private:
	string modelPath;
	void loadTexture(en_textureType type);
	unsigned int bindTexture(en_textureType type,string path);
};

