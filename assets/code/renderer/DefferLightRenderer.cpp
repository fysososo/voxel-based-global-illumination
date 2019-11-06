#include<stdafx.h>
#include "DefferLightRenderer.h"

void DefferLightRender::SetMVP(shared_ptr<Program> prog)
{
	//����projection����
	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glm::mat4 projectionM = glm::perspective(glm::radians(Camera::Active()->Zoom), (float)width / (float)height, 0.1f, 100.0f);
	prog->setMat4("projection", projectionM);

	//����view����
	glm::mat4 viewM = Camera::Active()->GetViewMatrix();
	prog->setMat4("view", viewM);

	//����model����
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, glm::vec3(0.0f, 0.0f, 0.0f));
	modelM = glm::scale(modelM, glm::vec3(1.0f, 1.0f, 1.0f));
	prog->setMat4("model", modelM);
}

void DefferLightRender::Render()
{
	SetAsActive();
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//����������ɫ
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//�����ɫ�������Ȼ���

	//���뼸����Ⱦ��Ŀ�껺��
	GLuint gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// - λ����ɫ����
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		// - ������ɫ����
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - ��ɫ + ������ɫ����
	glGenTextures(1, &gColorSpec);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

	//�ֲڶ�
	glGenTextures(1, &gRoughness);
	glBindTexture(GL_TEXTURE_2D, gRoughness);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 800, 600, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gRoughness, 0);

	//������
	glGenTextures(1, &gMetalness);
	glBindTexture(GL_TEXTURE_2D, gMetalness);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 800, 600, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMetalness, 0);

	// - ����OpenGL���ǽ�Ҫʹ��(֡�����)������ɫ������������Ⱦ
	GLuint attachments[5] = { 
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1, 
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4
	};

	for (auto& model : AssetsManager::Instance()->models) {
		for (auto& mesh : model.second->meshes) {
			auto& progGPass = AssetsManager::Instance()->programs["GPass"];
			progGPass->Use();
			SetMVP(progGPass);

			//����������λ��
			glUniform1i(glGetUniformLocation(progGPass->getID(), "NormalMap"), 0);
			glUniform1i(glGetUniformLocation(progGPass->getID(), "AlbedoMap"), 1);
			glUniform1i(glGetUniformLocation(progGPass->getID(), "RoughnessMap"), 2);
			glUniform1i(glGetUniformLocation(progGPass->getID(), "MetalnessMap"), 3);

			//��������
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh->material->normalMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mesh->material->albedoMap);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, mesh->material->roughnessMap);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, mesh->material->metalnessMap);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDrawElements(GL_TRIANGLES, mesh->Draw(), GL_UNSIGNED_INT, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glDrawBuffers(5, attachments);
			glBindVertexArray(0);
		}
	}

	//auto& proglightPass = AssetsManager::Instance()->programs["lightPass"];
	//proglightPass->Use();

	//SetMVP(proglightPass);

	////����ģ��
	//for (auto& model : AssetsManager::Instance()->models) {
	//	model.second->Draw();
	//}
	
}

void DefferLightRender::SetMaterialUniforms()
{
	this->material = material;
	//��ȡ����������Ϣ��renderer

#pragma region �����ӳ���Ⱦ�ļ��ν׶ε�uniform ����
	auto& progGPass = AssetsManager::Instance()->programs["GPass"];

#pragma endregion

#pragma region �����ӳ���Ⱦ�Ĺ��ս׶ε�uniform ����
	auto& progLight = AssetsManager::Instance()->programs["lightPass"];
	/*
	//��һ�㣺���ط�������
	glUniform1i(glGetUniformLocation(progLight->getID(), "voxelNormal"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, progVoxel.normal3DMap);

	//�ڶ��㣺���ط��ն�����
	glUniform1i(glGetUniformLocation(progLight->getID(), "voxelRadiance"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, progVoxel.radiance3DMapWithMipMap);

	//�����㣺��������������
	glUniform1i(glGetUniformLocation(progLight->getID(), "voxelIOR"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, progVoxel.radiance3DMapWithMipMap);
	*/
	
#pragma endregion
	
}
