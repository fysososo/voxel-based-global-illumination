#include "stdafx.h"
#include "voxelization.h"

void VoxelizationRenderer::Render()
{
	SetAsActive();

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置清屏颜色
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓存和深度缓存

	//使用体素化着色器程序
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];
	prog->Use();

	//设置MVP
	SetMVP(prog);

	
	//使用默认着色器程序
	auto& prog2 = AssetsManager::Instance()->programs["Default"];
	prog2->Use();

	//设置MVP
	SetMVP(prog2);

	//绘制模型
	for (auto& model : AssetsManager::Instance()->models) {
		model.second->Draw();
	}


}

void VoxelizationRenderer::SetMaterialUniforms(Material& material)
{
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];

	//绑定漫反射纹理
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(prog->getID(), "texture_diffuse"), 0);
	glBindTexture(GL_TEXTURE_2D, material.albedoMap);
}

void VoxelizationRenderer::SetMVP(shared_ptr<Program> prog)
{
	//传递projection矩阵
	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glm::mat4 projectionM = glm::perspective(glm::radians(Camera::Active()->Zoom), (float)width / (float)height, 0.1f, 100.0f);
	prog->setMat4("projection", projectionM);

	//传递view矩阵
	glm::mat4 viewM = Camera::Active()->GetViewMatrix();
	prog->setMat4("view", viewM);

	//传递model矩阵
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, glm::vec3(0.0f, -1.75f, 0.0f));
	modelM = glm::scale(modelM, glm::vec3(0.2f, 0.2f, 0.2f));
	prog->setMat4("model", modelM);
}
