#pragma once
#include <stdafx.h>

#include<../assets/code/shader/program.h>
class Material
{

public:

	enum en_textureType {
		en_TEXTURE_NORMAL = 0,
		en_TEXTURE_ROUGHNESS,
		en_TEXTURE_METANESS,
		en_TEXTURE_ALBEDO,
		en_TEXTURE_EMISSION,
		en_TEXTURE_SPECULAR
	};

	string matName;

	float roughness;
	float IOR;
	float metalness;
	float shininess;
	glm::vec3 F0;
	glm::vec3 albedo;
	glm::vec3 emission;
	glm::vec3 specular;
	
	unsigned int normalMap;
	unsigned int roughnessMap;
	unsigned int metalnessMap;
	unsigned int albedoMap;
	unsigned int emissionMap;
	unsigned int specularMap;


	void loadMaterial(string path);
	void BindMap(shared_ptr<Program> prog, GLenum textureSlot, en_textureType mapType);
	Material();
	~Material();

private:
	string modelPath;
	void loadTexture(en_textureType type);
	unsigned int bindTexture(en_textureType type,string path);
};

