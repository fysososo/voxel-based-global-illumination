#include<stdafx.h>
#include "DefferLightRenderer.h"

void DefferLightRender::SetMVP(shared_ptr<Program> prog)
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
	modelM = glm::translate(modelM, glm::vec3(0.0f, 0.0f, 0.0f));
	modelM = glm::scale(modelM, glm::vec3(1.0f, 1.0f, 1.0f));
	prog->setMat4("model", modelM);
}

void DefferLightRender::Render()
{
	SetAsActive();

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置清屏颜色
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓存和深度缓存

	for (auto& model : AssetsManager::Instance()->models) {
		for (auto& mesh : model.second->meshes) {
			auto& progGPass = AssetsManager::Instance()->programs["GPass"];
			progGPass->Use();
			SetMVP(progGPass);

			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓存和深度缓存
			//设置纹理
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mesh->material->normalMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mesh->material->albedoMap);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, mesh->material->roughnessMap);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, mesh->material->metalnessMap);

			glDrawElements(GL_TRIANGLES, mesh->Draw(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			auto& progLight = AssetsManager::Instance()->programs["lightPass"];
			progLight->Use();
			SetMVP(progLight);
			progLight->setVec3("viewPos", Camera::Active()->Position);
			progLight->setVec3("lightPos", glm::vec3(2.0f, 2.0f, 0.0f));

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓存和深度缓存

			//设置纹理
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

			if (quadVAO == 0)
			{
				GLfloat quadVertices[] = {
					// Positions        // Texture Coords
					-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
					-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
					1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
					1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
				};
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
			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glBindVertexArray(0);
		}
	}
	
}

void DefferLightRender::SetMaterialUniforms()
{

#pragma region 设置延迟渲染的几何阶段的uniform 属性

	//获取存有体素信息的renderer
	auto& progGPass = AssetsManager::Instance()->programs["GPass"];
	progGPass->Use();

	//设置纹理插槽位置
	glUniform1i(glGetUniformLocation(progGPass->getID(), "NormalMap"), 0);
	glUniform1i(glGetUniformLocation(progGPass->getID(), "AlbedoMap"), 1);
	glUniform1i(glGetUniformLocation(progGPass->getID(), "RoughnessMap"), 2);
	glUniform1i(glGetUniformLocation(progGPass->getID(), "MetalnessMap"), 3);

	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	//位置
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - 法线颜色缓冲
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - 颜色 + 镜面颜色缓冲
	glGenTextures(1, &gColorSpec);
	glBindTexture(GL_TEXTURE_2D, gColorSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

	//粗糙度
	glGenTextures(1, &gRoughness);
	glBindTexture(GL_TEXTURE_2D, gRoughness);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 800, 600, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gRoughness, 0);

	//金属度
	glGenTextures(1, &gMetalness);
	glBindTexture(GL_TEXTURE_2D, gMetalness);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 800, 600, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gMetalness, 0);

	// - 告诉OpenGL我们将要使用(帧缓冲的)哪种颜色附件来进行渲染
	GLuint attachments[5] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4
	};
	glDrawBuffers(5, attachments);

#pragma endregion

#pragma region 设置延迟渲染的4照阶段的uniform 属性
	auto& progLight = AssetsManager::Instance()->programs["lightPass"];
	progLight->Use();
	//设置纹理插槽位置
	glUniform1i(glGetUniformLocation(progLight->getID(), "gPosition"), 0);
	glUniform1i(glGetUniformLocation(progLight->getID(), "gNormal"), 1);
	glUniform1i(glGetUniformLocation(progLight->getID(), "gRoughness"), 2);
	glUniform1i(glGetUniformLocation(progLight->getID(), "gMetalness"), 3);
	glUniform1i(glGetUniformLocation(progLight->getID(), "gAbledo"), 4);
	/*
	//第一层：体素法线纹理
	glUniform1i(glGetUniformLocation(progLight->getID(), "voxelNormal"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, progVoxel.normal3DMap);

	//第二层：体素辐照度纹理
	glUniform1i(glGetUniformLocation(progLi//t->getID(), "voxelRadiance"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, progVoxel.radiance3DMapWithMipMap);

	//第三层：体素折射率纹理
	glUniform1i(glGetUniformLocation(progLight->getID(), "voxelIOR"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, progVoxel.radiance3DMapWithMipMap);
	*/
	
#pragma endregion
	
}
