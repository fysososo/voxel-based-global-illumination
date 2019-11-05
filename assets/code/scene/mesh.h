#pragma once
#include<stdafx.h>
#include "material.h"

class Mesh {
public:

	shared_ptr<Material> material;//材质

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, shared_ptr<Material> material);

	int Draw();

	~Mesh();

private:
	GLuint VAO, VBO, EBO;//各种缓冲对象
	vector<Vertex> vertices;//顶点
	vector<GLuint> indices;//索引
	//设置各种数据与顶点着色器的传输规则，并存于VAO中，用于之后绘制
	void setupMesh();
};
