#pragma once
#include "renderer.h"
#include "../scene/material.h"

class VoxelizationRenderer : public Renderer
{
private:
	void SetMVP(shared_ptr<Program> prog);

public:
	virtual void Render() override;
	virtual void SetMaterialUniforms(Material& material) override;

};

