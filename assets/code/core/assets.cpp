#include <stdafx.h>
#include "assets.h"

//程序类
#include "../program/p_voxelization.h"

//渲染器类
#include "../renderer/r_voxelization.h"

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
	cameras["FPS"] = make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
	cameras["FPS"]->SetAsActive();//激活

	//加载模型
	models["test"] = make_shared<Model>("assets/model/test/nanosuit.obj");
	
	//创建程序，并附加shader
	programs["Voxelization"] = make_shared<VoxelizationProgram>();
	programs["Voxelization"]->AttachShader(GL_VERTEX_SHADER, "assets/code/shader/test.vert");
	programs["Voxelization"]->AttachShader(GL_FRAGMENT_SHADER, "assets/code/shader/test.frag");
	
	
	//链接生成所有shader程序
	for (auto& prog : programs)
	{
		prog.second->Link();
	}

	//创建渲染器
	renderers["Voxelization"] = make_shared<VoxelizationRenderer>();
}

AssetsManager::~AssetsManager()
{

}

void AssetsManager::Terminate()
{
	delete Instance().release();
}