#include<stdafx.h>
#include "mesh.h"
#include "material.h"

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, shared_ptr<Material> material)
{
	this->vertices = vertices;
	this->indices = indices;
	this->material = material;

	setupMesh();//设置网格
}

int Mesh::Draw()
{
	//绘制mesh
	glBindVertexArray(VAO);
	return indices.size();	
}

Mesh::~Mesh()
{
}

void Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);


	//把顶点属性传送到VBO中
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	//设置顶点着色器如何获取顶点数据
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	//设置顶点着色器如何获取法线数据
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	//设置顶点着色器如何获取纹理坐标数据
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	//设置顶点着色器如何获取切线数据
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	//设置顶点着色器如何获取副切线数据
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	//把索引数据传送到EBO中
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


	glBindVertexArray(0);
}