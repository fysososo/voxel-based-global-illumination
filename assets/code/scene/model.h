#pragma once
#include "mesh.h"

class Model
{
public:
	BoundingBox boundingBox;//包围盒

	//传入模型文件路径，载入模型
	Model(string const& path, bool gamma = false);

	//绘制模型
	void Draw();

	//绘制包围盒
	void DrawBoundingBox();
	
	~Model();

private:
	const aiScene* scene;//场景指针
	string directory;//模型文件路径
	vector<shared_ptr<Material>> materials;//所有材质（纹理指针）数据
	vector<shared_ptr<Mesh>> meshes;//所有网格（顶点、索引、材质指针）数据
	bool gammaCorrection;//伽马修正
	GLuint VAO;//包围盒的VAO

	//加载模型
	void loadModel(string const& path);

	//载入网格
	void loadMeshes();

	//设置包围盒
	void SetupBoundingBox();
	
};

