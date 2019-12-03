#include <stdafx.h>
#include "assets.h"

//��Ⱦ����
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
	//�������
	cameras["FPS"] = make_shared<Camera>(glm::vec3(0.0f, 0.0f, 15.0f));
	cameras["FPS"]->SetAsActive();//����

	//���صƹ�
	pointLights.push_back(make_shared<Pointlight>(glm::vec3(10.0f, 10.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 25.0f));
	pointLights.push_back(make_shared<Pointlight>(glm::vec3(-10.0f, 10.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 25.0f));

	//����ģ��
	models["sphere1"] = make_shared<Model>("assets/model/sphere/sphere.obj", glm::vec3(-4.0f,0.0f,0.0f));
	models["sphere2"] = make_shared<Model>("assets/model/sphere/sphere.obj", glm::vec3(4.0f,0.0f,0.0f));

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
	//������������shader����
	for (auto& prog : programs)
	{
		prog.second->Link();
	}

	//������Ⱦ��
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