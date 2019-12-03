#pragma once
#include<stdafx.h>
#include "mesh.h"

class Model
{
public:
	BoundingBox boundingBox;//��Χ��

	glm::vec3 position;

	//����ģ���ļ�·��������ģ��
	Model(string const& path, glm::vec3 position, bool gamma = false);

	//���ư�Χ��
	void DrawBoundingBox();

	void Draw();
	
	vector<shared_ptr<Material>> materials;//���в��ʣ�����ָ�룩����
	vector<shared_ptr<Mesh>> meshes;//�������񣨶��㡢����������ָ�룩����

	~Model();

private:
	const aiScene* scene;//����ָ��
	string directory;//ģ���ļ�·��
	bool gammaCorrection;//٤������
	GLuint VAO;//��Χ�е�VAO

	//����ģ��
	void loadModel(string const& path, glm::vec3 postion);

	//��������
	void loadMeshes();

	//���ð�Χ��
	void SetupBoundingBox();
	
};

