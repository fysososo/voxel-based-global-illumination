#pragma once
#include "renderer.h"

class MipmapRenderBase : public Renderer {
public:
	GLuint voxelBaseMipmap[6];
	virtual void Render() override;
	virtual void SetMaterialUniforms() override;
};

class MipmapRenderVolume : public Renderer {
	virtual void Render() override;
	virtual void SetMaterialUniforms() override;
};