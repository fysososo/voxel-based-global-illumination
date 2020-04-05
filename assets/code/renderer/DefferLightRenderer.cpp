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
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//����������ɫ
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//�����ӿ�
	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glViewport(0, 0, width, height);
	//���Ŀ�����ɫ�������Ȼ���
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//���ؼ��ν׶�program
	auto& progGPass = AssetsManager::Instance()->programs["GPass"];
	progGPass->Use();
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
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
			mesh->material->BindMap(progGPass, GL_TEXTURE4, Material::en_TEXTURE_EMISSION);
			mesh->material->BindMap(progGPass, GL_TEXTURE5, Material::en_TEXTURE_SPECULAR);

			//���ò�����Ϣ
			progGPass->setFloat("metalness", mesh->material->metalness);
			progGPass->setFloat("shininess", mesh->material->shininess);
			progGPass->setFloat("roughness", mesh->material->roughness);
			progGPass->setVec3("albedo", mesh->material->albedo);
			progGPass->setVec3("specular", mesh->material->specular);
			progGPass->setVec3("emission", mesh->material->emission);
			progGPass->setFloat("F0", mesh->material->F0.x);
			progGPass->setFloat("KD", 0.4f);
			progGPass->setFloat("IOR", mesh->material->IOR);
			mesh->Draw();
		}
	}

	//������ȾĿ��Ϊ����
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	
	//�����ӿ�
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glViewport(0, 0, width, height);
	//���Ŀ�����ɫ�������Ȼ���
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//���ع��ս׶�program
	auto& progLight = AssetsManager::Instance()->programs["lightPass"];
	progLight->Use();
	progLight->setVec3("viewPos", Camera::Active()->Position);

	//���õ��Դ
	progLight->setInt("lightCount", (int)AssetsManager::Instance()->pointLights.size());
	for (int i = 0; i < AssetsManager::Instance()->pointLights.size(); i++) {
		auto& light = AssetsManager::Instance()->pointLights[i];
		progLight->setVec3("pointLight[" + to_string(i) + "].position", light->position);
		progLight->setVec3("pointLight[" + to_string(i) + "].ambient", light->Ambient());
		progLight->setVec3("pointLight[" + to_string(i) + "].diffuse", light->Diffuse());
		progLight->setVec3("pointLight[" + to_string(i) + "].specular", light->Specular());
		progLight->setFloat("pointLight[" + to_string(i) + "].attenuation.constant", 1.0f);
		progLight->setFloat("pointLight[" + to_string(i) + "].attenuation.linear", 0.2f);
		progLight->setFloat("pointLight[" + to_string(i) + "].attenuation.quadratic", 0.08f);
	}

	//��������
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gRoughness);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gMetalness);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, gEmission);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, gSpecular);

	//����������
	auto& voxelRender = *static_cast<VoxelizationRenderer*>(AssetsManager::Instance()->renderers["Voxelization"].get());
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_3D, voxelRender.voxelRadiance);

	//�������������Ϣ
	progLight->setFloat("voxelScale", 1.0f/(voxelRender.voxelSize*voxelRender.dimension));
	progLight->setFloat("voxelSize", voxelRender.voxelSize);
	progLight->setFloat("bounceStrength", 1.0f);
	progLight->setFloat("maxTracingDistanceGlobal", 1.0f);
	progLight->setFloat("samplingFactor", 0.5f);
	progLight->setInt("volumeDimension", voxelRender.dimension);
	progLight->setVec3("worldMaxPoint", voxelRender.sceneBoundingBox.MaxPoint);
	progLight->setVec3("worldMinPoint", voxelRender.sceneBoundingBox.MinPoint);

	//����任
	glm::mat4 inverseView = glm::inverse(Camera::Active()->GetViewMatrix()); 
	glm::mat4 inverseProjection = glm::inverse(glm::perspective(glm::radians(Camera::Active()->Zoom), (float)width / (float)height, 0.1f, 100.0f));
	progLight->setMat4("inverseProjectionView", inverseView*inverseProjection);
	for (int i = 0; i < 6; i++) {
		glActiveTexture(GL_TEXTURE9+i);
		glBindTexture(GL_TEXTURE_3D, voxelRender.voxelAnisoMipmap[i]);
	}

	glBindImageTexture(0, gDebug, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	//������Ⱦ
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
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
	glUniform1i(glGetUniformLocation(progGPass->getID(), "EmissionMap"), 4);
	glUniform1i(glGetUniformLocation(progGPass->getID(), "SpecularMap"), 5);

	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	//λ��
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - ������ɫ����
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - ��ɫ + ������ɫKD
	glGenTextures(1, &gColorSpec);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
	
	//�԰l��
	glGenTextures(1, &gEmission);
	glBindTexture(GL_TEXTURE_2D, gEmission);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gEmission, 0);

	//���淴��
	glGenTextures(1, &gSpecular);
	glBindTexture(GL_TEXTURE_2D, gSpecular);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gSpecular, 0);

	//Debug
	glGenTextures(1, &gDebug);
	glBindTexture(GL_TEXTURE_2D, gDebug);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// - ����OpenGL���ǽ�Ҫʹ��(֡�����)������ɫ������������Ⱦ
	GLuint attachments[6] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5
	};
	glDrawBuffers(6, attachments);

#pragma endregion

#pragma region �����ӳ���Ⱦ�Ĺ��ս׶ε�uniform ����
	auto& progLight = AssetsManager::Instance()->programs["lightPass"];
	progLight->Use();

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
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 800, 600);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
