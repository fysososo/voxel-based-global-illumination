#include"MipmapingRender.h"
#include"voxelization.h"

void MipmapRenderBase::Render()
{
	auto& voxelRender = *static_cast<VoxelizationRenderer*>(AssetsManager::Instance()->renderers["Voxelization"].get());
	
	auto& progMipmapBase = AssetsManager::Instance()->programs["MipmapBase"];
	progMipmapBase->Use();
	progMipmapBase->setInt("mipDimension", voxelRender.dimension);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_3D, voxelRender.albedo);
}

void MipmapRenderBase::SetMaterialUniforms()
{

}

void MipmapRenderVolume::Render()
{
}

void MipmapRenderVolume::SetMaterialUniforms()
{
}
