#pragma once
#include <stdafx.h>

#include "mesh.h"
#include "../render/program.h"


class Model
{
public:
	//传入模型文件路径，载入模型
	Model(string const& path, bool gamma = false);

	//绘制模型
	void Draw(Program& shader);

	~Model();

private:
	const aiScene* scene;//场景指针
	string directory;//模型文件路径
	vector<shared_ptr<Texture>> textures;//所有纹理
	vector<shared_ptr<Material>> materials;//所有材质（纹理指针）数据
	vector<shared_ptr<Mesh>> meshes;//所有网格（顶点、索引、材质指针）数据
	bool gammaCorrection;//伽马修正

	//加载模型
	void loadModel(string const& path);

	//载入材质，同时不重复地载入贴图
	void loadMaterials();

	//载入网格
	void loadMeshes();

	//为材质读取纹理数据（确保不重复加载）
	vector<shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
	
	//从文件中获取纹理数据
	GLuint TextureFromFile(const char* path, const string& directory, bool gamma = false);
	
};

