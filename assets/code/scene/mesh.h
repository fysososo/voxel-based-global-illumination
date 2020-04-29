#pragma once
#include<stdafx.h>

class Mesh {
public:

	shared_ptr<Material> material;//����

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, shared_ptr<Material> material);

	int Draw();

	~Mesh();

private:
	GLuint VAO, VBO, EBO;//���ֻ������
	vector<Vertex> vertices;//����
	vector<GLuint> indices;//����
	//���ø��������붥����ɫ���Ĵ�����򣬲�����VAO�У�����֮�����
	void setupMesh();
};
