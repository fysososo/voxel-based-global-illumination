#include <stdafx.h>
#include "assets.h"

//渲染器类
#include "../renderer/voxelization.h"
#include "../renderer/DefferLightRenderer.h"

unique_ptr<AssetsManager>& AssetsManager::Instance()
{
	static unique_ptr<AssetsManager> instance = nullptr;

	if (!instance)
	{
		instance.reset(new AssetsManager());
	}

	return instance;
}

AssetsManager::AssetsManager()
{
	//加载相机
	cameras["FPS"] = make_shared<Camera>(glm::vec3(0.0f, 0.0f, -4.0f));
	cameras["FPS"]->SetAsActive();//激活

	//加载灯光
	pointLights.push_back(make_shared<Pointlight>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 15.0f));
	//pointLights.push_back(make_shared<Pointlight>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 15.0f));
	//加载模型
	models["sphere1"] = make_shared<Model>("assets/model/sphere/sphere.obj", glm::vec3(-1.5f,0.0f,3.0f));
	models["sphere2"] = make_shared<Model>("assets/model/sphere/sphere.obj", glm::vec3(1.5f,0.0f,3.0f));
	models["cornell"] = make_shared<Model>("assets/model/cornell-box/cornell_box_EMPTY.obj", glm::vec3(0.0f, -2.0f, 0.0f));
	
	programs["Voxelization"] = make_shared<Program>();
	programs["Voxelization"]->AttachShader(GL_VERTEX_SHADER, "assets/code/shader/voxelization.vert");
	programs["Voxelization"]->AttachShader(GL_GEOMETRY_SHADER, "assets/code/shader/voxelization.geom");
	programs["Voxelization"]->AttachShader(GL_FRAGMENT_SHADER, "assets/code/shader/voxelization.frag");

	programs["WhiteLine"] = make_shared<Program>();
	programs["WhiteLine"]->AttachShader(GL_VERTEX_SHADER, "assets/code/shader/whiteLine.vert");
	programs["WhiteLine"]->AttachShader(GL_FRAGMENT_SHADER, "assets/code/shader/whiteLine.frag");

	programs["DrawVoxel"] = make_shared<Program>();
	programs["DrawVoxel"]->AttachShader(GL_VERTEX_SHADER, "assets/code/shader/drawVoxel.vert");
	programs["DrawVoxel"]->AttachShader(GL_GEOMETRY_SHADER, "assets/code/shader/drawVoxel.geom");
	programs["DrawVoxel"]->AttachShader(GL_FRAGMENT_SHADER, "assets/code/shader/drawVoxel.frag");

	programs["GPass"] = make_shared<Program>();
	programs["GPass"]->AttachShader(GL_VERTEX_SHADER, "assets/code/shader/GPass.vert");
	programs["GPass"]->AttachShader(GL_FRAGMENT_SHADER, "assets/code/shader/GPass.frag");

	programs["lightPass"] = make_shared<Program>();
	programs["lightPass"]->AttachShader(GL_VERTEX_SHADER, "assets/code/shader/lightPass.vert");
	programs["lightPass"]->AttachShader(GL_FRAGMENT_SHADER, "assets/code/shader/lightPass.frag");
	
	programs["anisoMipmapFirst"] = make_shared<Program>();
	programs["anisoMipmapFirst"]->AttachShader(GL_COMPUTE_SHADER, "assets/code/shader/anisoMipmapFirst.comp");

	programs["anisoMipmapOthers"] = make_shared<Program>();
	programs["anisoMipmapOthers"]->AttachShader(GL_COMPUTE_SHADER, "assets/code/shader/anisoMipmapOthers.comp");

	//链接生成所有shader程序
	for (auto& prog : programs)
	{
		prog.second->Link();
	}

	//创建渲染器
	renderers["Voxelization"] = make_shared<VoxelizationRenderer>();
	renderers["DefferLight"] = make_shared<DefferLightRender>();
}

AssetsManager::~AssetsManager()
{

}

void AssetsManager::Terminate()
{
	delete Instance().release();
}