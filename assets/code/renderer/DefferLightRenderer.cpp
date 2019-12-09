#include<stdafx.h>
#include "DefferLightRenderer.h"
#include "voxelization.h"

void DefferLightRender::SetVP(shared_ptr<Program> prog)
{
	//����projection����
	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glm::mat4 projectionM = glm::perspective(glm::radians(Camera::Active()->Zoom), (float)width / (float)height, 0.1f, 100.0f);
	prog->setMat4("projection", projectionM);

	//����view����
	glm::mat4 viewM = Camera::Active()->GetViewMatrix();
	prog->setMat4("view", viewM);
}

void DefferLightRender::setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model)
{
	//����model����
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, model->position);
	modelM = glm::scale(modelM, glm::vec3(1.0f, 1.0f, 1.0f));
	prog->setMat4("model", modelM);
}

void DefferLightRender::Render()
{

	SetAsActive();

	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//����������ɫ
	glClearDepth(1.0f);

	//���ü��ν׶���ȾĿ��
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	//���Ŀ�����ɫ�������Ȼ���
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//���ؼ��ν׶�program
	auto& progGPass = AssetsManager::Instance()->programs["GPass"];
	progGPass->Use();
	//�������
	SetVP(progGPass);
	//��Ⱦģ��
	for (auto& model : AssetsManager::Instance()->models) {
		setModelMat(progGPass, model.second);
		for (auto& mesh : model.second->meshes) {
			//��������
			mesh->material->BindMap(progGPass, GL_TEXTURE0, Material::en_TEXTURE_NORMAL);
			mesh->material->BindMap(progGPass, GL_TEXTURE1, Material::en_TEXTURE_ALBEDO);
			mesh->material->BindMap(progGPass, GL_TEXTURE2, Material::en_TEXTURE_ROUGHNESS);
			mesh->material->BindMap(progGPass, GL_TEXTURE3, Material::en_TEXTURE_METANESS);

			//���ò�����Ϣ
			progGPass->setFloat("metalness", mesh->material->metalness);
			progGPass->setFloat("roughness", mesh->material->roughness);
			progGPass->setVec3("albedo", mesh->material->albedo);
			progGPass->setFloat("F0", mesh->material->F0.x);
			progGPass->setFloat("KD", 0.4f);
			progGPass->setFloat("IOR", mesh->material->IOR);
			mesh->Draw();
		}
	}
	glBindVertexArray(0);

	//���ع��ս׶�program
	auto& progLight = AssetsManager::Instance()->programs["lightPass"];
	progLight->Use();
	progLight->setVec3("viewPos", Camera::Active()->Position);

	//���õ��Դ
	progLight->setInt("lightCount", (int)AssetsManager::Instance()->pointLights.size());
	for (int i = 0; i < AssetsManager::Instance()->pointLights.size(); i++) {
		auto& light = AssetsManager::Instance()->pointLights[i];
		progLight->setVec3("pointLight[" + to_string(i) + "].position", light->position);
		progLight->setVec3("pointLight[" + to_string(i) + "].color", light->color);
		progLight->setFloat("pointLight[" + to_string(i) + "].intensity", light->intensity);
	}
	//������ȾĿ��Ϊ����
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//��������
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gRoughness);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gMetalness);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);

	//����������
	auto& voxelRender = *static_cast<VoxelizationRenderer*>(AssetsManager::Instance()->renderers["Voxelization"].get());
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_3D, voxelRender.voxelRadiance);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_3D, voxelRender.normal);

	//�������������Ϣ
	progLight->setFloat("voxelScale", voxelRender.voxelSize);
	progLight->setInt("volumeDimension", voxelRender.dimension);
	progLight->setVec3("worldMaxPoint", voxelRender.sceneBoundingBox.MaxPoint);
	progLight->setVec3("worldMinPoint", voxelRender.sceneBoundingBox.MinPoint);


	for (int i = 0; i < 6; i++) {
		glActiveTexture(GL_TEXTURE8+i);
		glBindTexture(GL_TEXTURE_3D, voxelRender.voxelAnisoMipmap[i]);
	}

	//������Ⱦ
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	
}

void DefferLightRender::SetMaterialUniforms()
{

#pragma region �����ӳ���Ⱦ�ļ��ν׶ε�uniform ����

	//��ȡ����������Ϣ��renderer
	auto& progGPass = AssetsManager::Instance()->programs["GPass"];
	progGPass->Use();

	//����������λ��
	glUniform1i(glGetUniformLocation(progGPass->getID(), "NormalMap"), 0);
	glUniform1i(glGetUniformLocation(progGPass->getID(), "AlbedoMap"), 1);
	glUniform1i(glGetUniformLocation(progGPass->getID(), "RoughnessMap"), 2);
	glUniform1i(glGetUniformLocation(progGPass->getID(), "MetalnessMap"), 3);

	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	//λ��
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

	// - ��ɫ + ������ɫKD
	glGenTextures(1, &gColorSpec);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

	//�ֲڶ�+������+F0+IOR
	glGenTextures(1, &gRoughness);
	glBindTexture(GL_TEXTURE_2D, gRoughness);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gRoughness, 0);

	// - ����OpenGL���ǽ�Ҫʹ��(֡�����)������ɫ������������Ⱦ
	GLuint attachments[4] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3
	};
	glDrawBuffers(4, attachments);

#pragma endregion

#pragma region �����ӳ���Ⱦ�Ĺ��ս׶ε�uniform ����
	auto& progLight = AssetsManager::Instance()->programs["lightPass"];
	progLight->Use();

	//����������λ��
	glUniform1i(glGetUniformLocation(progLight->getID(), "gPosition"), 0);
	glUniform1i(glGetUniformLocation(progLight->getID(), "gNormal"), 1);
	glUniform1i(glGetUniformLocation(progLight->getID(), "gRoughness"), 2);
	glUniform1i(glGetUniformLocation(progLight->getID(), "gMetalness"), 3);
	glUniform1i(glGetUniformLocation(progLight->getID(), "gAbledo"), 4);

	//��������
	glUniform1i(glGetUniformLocation(progLight->getID(), "voxelRadiance"), 5);
	glUniform1i(glGetUniformLocation(progLight->getID(), "voxelNormal"), 6);
	glUniform1i(glGetUniformLocation(progLight->getID(), "voxelIOR"), 7);
	for (int i = 0; i < 6; i++) {
		glUniform1i(glGetUniformLocation(progLight->getID(), ("voxelTexMipmap[" + to_string(i)+ "]").c_str()), 8+i);
	}

	if (quadVAO == 0)
	{
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}

#pragma endregion

	//������Ȼ���
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 800, 600);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}
