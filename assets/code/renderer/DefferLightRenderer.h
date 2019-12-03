#pragma once
#include<stdafx.h>
#include "renderer.h"
#include "../scene/material.h"
class DefferLightRender :
	public Renderer
{
private:
	GLfloat quadVertices[20] = {
		// Positions        // Texture Coords
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	void SetVP(shared_ptr<Program> prog);
	void setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model);
	shared_ptr<Material> material;
public:
	virtual void Render() override;
	virtual void SetMaterialUniforms() override;
	GLuint gPosition, gNormal, gColorSpec, gRoughness, gMetalness;
	GLuint gBuffer;
	GLuint quadVAO, quadVBO;
};