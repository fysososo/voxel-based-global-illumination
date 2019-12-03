#pragma once
#include "renderer.h"
#include "../scene/material.h"

class VoxelizationRenderer : public Renderer
{
private:

	GLuint VAO_drawVoxel;

	void SetMVP_freeMove(shared_ptr<Program> prog);
	void SetMVP_ortho(shared_ptr<Program> prog, BoundingBox& boundingBox);
	void Set3DTexture();//����3d����
	void DrawVoxel(shared_ptr<Model> model);//��������
	void setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model);
	void drawSceneBoundingBox();

public:
	virtual void Render() override;
	virtual void SetMaterialUniforms() override;
	VoxelizationRenderer();
	GLuint albedo;//����������ɫ��3D����
	GLfloat gridSize;//����ߴ磨=�Ӿ�����ߣ�
	GLuint dimension;//һ�����ص�����
	GLfloat voxelSize;//��λ���سߴ�
	BoundingBox sceneBoundingBox;
};

