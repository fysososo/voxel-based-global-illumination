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
	void Set3DTexture();//设置3d纹理
	void setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model);
	void GenerateMipmapOthers();
	void GenerateMipmapFirst(GLuint baseTexture);
	void InjectRadiance();
	void RadiancePropagation();

public:
	virtual void Render() override;
	virtual void SetMaterialUniforms() override;
	VoxelizationRenderer();
	GLuint albedo;//储存体素颜色的3D纹理
	GLuint normal;//储存体素法线的3D纹理
	GLuint emission;//储存体素自发光的3D纹理
	GLuint radiance;//储存体素辐照度的3D纹理
	GLuint roughness;//储存体素粗糙度的3D纹理
	GLuint metalness;//储存体素金属度的3D纹理
	GLuint debug_comp;//調試用
	GLuint debug_comp_injectRadiance;//調試用

	GLuint voxelRadiance;
	std::array<GLuint, 6> voxelAnisoMipmap;
	GLfloat gridSize;//网格尺寸（=视景体最长边）
	GLuint dimension;//一排体素的数量
	GLfloat voxelSize;//单位体素尺寸
	BoundingBox sceneBoundingBox;
};

