#pragma once
#include<array>
#include "renderer.h"
#include "../scene/material.h"

class VoxelizationRenderer : public Renderer
{
private:

	GLuint VAO_drawVoxel;

	void SetMVP_freeMove(shared_ptr<Program> prog);
	void SetMVP_ortho(shared_ptr<Program> prog, BoundingBox& boundingBox);
	void Set3DTexture();//����3d����
	void setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model);
	void GenerateMipmapOthers();
	void GenerateMipmapFirst(GLuint baseTexture);
	void InjectRadiance();
	void RadiancePropagation();

public:
	virtual void Render() override;
	virtual void SetMaterialUniforms() override;
	VoxelizationRenderer();
	GLuint albedo;//����������ɫ��3D����
	GLuint normal;//�������ط��ߵ�3D����
	GLuint emission;//���������Է����3D����
	GLuint radiance;//�������ط��նȵ�3D����
	GLuint roughness;//�������شֲڶȵ�3D����
	GLuint metalness;//�������ؽ����ȵ�3D����
	GLuint debug_comp;//�{ԇ��
	GLuint debug_comp_injectRadiance;//�{ԇ��

	GLuint voxelRadiance;
	std::array<GLuint, 6> voxelAnisoMipmap;
	GLfloat gridSize;//����ߴ磨=�Ӿ�����ߣ�
	GLuint dimension;//һ�����ص�����
	GLfloat voxelSize;//��λ���سߴ�
	BoundingBox sceneBoundingBox;
};

