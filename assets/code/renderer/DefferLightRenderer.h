#pragma once
#include<stdafx.h>
#include "renderer.h"
#include "../scene/material.h"
class DefferLightRender :
	public Renderer
{
private:
	void SetMVP(shared_ptr<Program> prog);
	shared_ptr<Material> material;
public:
	virtual void Render() override;
	virtual void SetMaterialUniforms() override;
};

