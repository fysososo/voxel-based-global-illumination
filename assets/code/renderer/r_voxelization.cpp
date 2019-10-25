#include "stdafx.h"
#include "r_voxelization.h"

void VoxelizationRenderer::Render()
{
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置清屏颜色
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓存和深度缓存

	//使用着色器程序
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];
	prog->Use();

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
	modelM = glm::translate(modelM, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	modelM = glm::scale(modelM, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	prog->setMat4("model", modelM);

	//绘制
	auto& model = AssetsManager::Instance()->models["test"];
	model->Draw(*prog);
#pragma endregion
}