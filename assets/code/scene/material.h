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
	void BindMap(shared_ptr<Program> prog, GLenum textureSlot, en_textureType mapType);
	Material();
	~Material();

private:
	string modelPath;
	void loadTexture(en_textureType type);
	unsigned int bindTexture(en_textureType type,string path);
};

