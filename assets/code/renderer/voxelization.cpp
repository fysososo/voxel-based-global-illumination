#include "stdafx.h"
#include "voxelization.h"

void VoxelizationRenderer::Render()
{
	SetAsActive();

	//ע�������Ϣ
	InjectRadiance();
	//����mipmap
	GenerateMipmapFirst(voxelRadiance);
	GenerateMipmapOthers();
	//��һ�������ش���
	RadiancePropagation();
	//����mipmap
	GenerateMipmapFirst(voxelRadiance);
	GenerateMipmapOthers();
}

void VoxelizationRenderer::SetMaterialUniforms()
{
	CalculateSceneBondingBox();

	//������ʵ���س���̶�����
	auto& progDrawVoxel = AssetsManager::Instance()->programs["DrawVoxel"];
	progDrawVoxel->Use();
	progDrawVoxel->setFloat("voxelSize", voxelSize);
	progDrawVoxel->setInt("dimension", dimension);
	progDrawVoxel->setVec3("boxMin", sceneBoundingBox.MinPoint);

	//����������ֱ�ӹ��ճ���̶�����
	auto& proginject = AssetsManager::Instance()->programs["injectRadiance"];
	proginject->Use();
	proginject->setFloat("traceShadowHit", 0.5f);

	//�������ػ���ɫ������̶�����
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];
	prog->Use();
	prog->setFloat("voxelScale", 1.0f / (voxelSize * dimension));
	prog->setVec3("worldMinPoint", sceneBoundingBox.MinPoint);
	prog->setUnsignedInt("dimension", dimension);
}

VoxelizationRenderer::VoxelizationRenderer()
{
	gridSize = 0;//����ߴ磨=�Ӿ�����ߣ�
	dimension = 256;//һ�����ص�����
	voxelSize = 0;//��λ���سߴ�
	glGenVertexArrays(1, &VAO_drawVoxel);
	Set3DTexture();
}

void VoxelizationRenderer::SetMVP_freeMove(shared_ptr<Program> prog)
{
	//����projection����
	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glm::mat4 projectionM = glm::perspective(glm::radians(Camera::Active()->Zoom), (float)width / (float)height, 0.1f, 1000.0f);
	prog->setMat4("matrices.projection", projectionM);

	//����view����
	glm::mat4 viewM = Camera::Active()->GetViewMatrix();
	prog->setMat4("matrices.view", viewM);
}

void VoxelizationRenderer::SetMVP_ortho(shared_ptr<Program> prog, BoundingBox& boundingBox)
{
	auto halfSize = gridSize / 2.0f;
	
	//projection����
	glm::mat4 projectionM = glm::ortho
		(-halfSize, halfSize, -halfSize, halfSize, 0.0f, gridSize);

	//view����
	glm::mat4 viewM[3];
	viewM[0] = glm::lookAt//����yzƽ��
		(boundingBox.Center + glm::vec3(halfSize, 0.0f, 0.0f),
		 boundingBox.Center,
		 glm::vec3(0.0f, 1.0f, 0.0f));
	viewM[1] = glm::lookAt//����xzƽ��
		(boundingBox.Center + glm::vec3(0.0f, halfSize, 0.0f),
		 boundingBox.Center,
		 glm::vec3(0.0f, 0.0f, -1.0f));
	viewM[2] = glm::lookAt//����xyƽ��
		(boundingBox.Center + glm::vec3(0.0f, 0.0f, halfSize),
		 boundingBox.Center,
		 glm::vec3(0.0f, 1.0f, 0.0f));


	prog->setMat4("viewProject[0]", projectionM * viewM[0]);
	prog->setMat4("viewProject[1]", projectionM * viewM[1]);
	prog->setMat4("viewProject[2]", projectionM * viewM[2]);
	prog->setMat4("viewProjectI[0]", inverse(projectionM * viewM[0]));
	prog->setMat4("viewProjectI[1]", inverse(projectionM * viewM[1]));
	prog->setMat4("viewProjectI[2]", inverse(projectionM * viewM[2]));

}

void VoxelizationRenderer::Set3DTexture()
{
	//����albedo 3D����
	glGenTextures(1, &albedo);
	glBindTexture(GL_TEXTURE_3D, albedo);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//����normal 3D����
	glGenTextures(1, &normal);
	glBindTexture(GL_TEXTURE_3D, normal);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//����emission 3D����
	glGenTextures(1, &emission);
	glBindTexture(GL_TEXTURE_3D, emission);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//����voxelRadiance 3D����
	glGenTextures(1, &voxelRadiance);
	glBindTexture(GL_TEXTURE_3D, voxelRadiance);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//����debug 3D����
	glGenTextures(1, &debug_comp);
	glBindTexture(GL_TEXTURE_3D, debug_comp);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_FLOAT, nullptr);

	//����6�������mipmap����
	for (int i = 0; i < 6; i++)
	{
		glGenTextures(1, &voxelAnisoMipmap[i]);
		glBindTexture(GL_TEXTURE_3D, voxelAnisoMipmap[i]);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
			dimension / 2, dimension / 2, dimension / 2,
			0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glGenerateMipmap(GL_TEXTURE_3D);
	}
}

void VoxelizationRenderer::GenerateMipmapOthers()
{
	//ʹ��mipmapOthers������ɫ��
	auto& prog = AssetsManager::Instance()->programs["anisoMipmapOthers"];
	prog->Use();

	GLint mipDimension = dimension / 4;
	GLint mipLevel = 0;

	while (mipDimension >= 1)
	{
		prog->setInt("mipDimension", mipDimension);
		prog->setInt("mipLevel", mipLevel);

		for (auto i = 0; i < voxelAnisoMipmap.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE6 + i);
			glBindTexture(GL_TEXTURE_3D, voxelAnisoMipmap[i]); 
			glBindImageTexture(i, voxelAnisoMipmap[i], mipLevel + 1, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		}

		auto workGroups = static_cast<unsigned>(glm::ceil(mipDimension / 8.0f));
		glDispatchCompute(workGroups, workGroups, workGroups);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		mipLevel++;
		mipDimension /= 2;
	}
}


void VoxelizationRenderer::GenerateMipmapFirst(GLuint baseTexture)
{
	//ʹ��mipmapFirst������ɫ��
	auto& prog = AssetsManager::Instance()->programs["anisoMipmapFirst"];
	prog->Use();

	GLint halfDimension = dimension / 2;
	prog->setInt("mipDimension", halfDimension);

	//�������������Խ��յ�һ��mipmap
	for (int i = 0; i < voxelAnisoMipmap.size(); ++i)
	{
		glBindImageTexture(i, voxelAnisoMipmap[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	}
	//��ԭʼ3D��������
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_3D, baseTexture);

	//��ʼ���㣬�õ����ŷ���ͬ�ĵ�һ��mipmap
	auto workGroups = static_cast<unsigned int>(ceil(halfDimension / 8.0f));
	glDispatchCompute(workGroups, workGroups, workGroups);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VoxelizationRenderer::InjectRadiance()
{
	auto& prog = AssetsManager::Instance()->programs["injectRadiance"];
	prog->Use();
	//��Albedo����
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, albedo);
	//��Normal����
	glBindImageTexture(0, normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	//��Emisson����
	glBindImageTexture(1, emission, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
	//���������
	glBindImageTexture(2, voxelRadiance, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	

	//���ù�Դ
	prog->setInt("lightCount", (int)AssetsManager::Instance()->pointLights.size());
	for (int i = 0; i < AssetsManager::Instance()->pointLights.size(); i++) {
		auto& light = AssetsManager::Instance()->pointLights[i];
		prog->setVec3("pointLight[" + to_string(i) + "].position", light->position);
		prog->setVec3("pointLight[" + to_string(i) + "].diffuse", light->Diffuse());
		prog->setFloat("pointLight[" + to_string(i) + "].attenuation.constant", light->constant);
		prog->setFloat("pointLight[" + to_string(i) + "].attenuation.linear", light->linear);
		prog->setFloat("pointLight[" + to_string(i) + "].attenuation.quadratic", light->quadratic);
	}
	//���������λ��
	prog->setVec3("viewPos", Camera::Active()->Position);

	//�������������Ϣ
	prog->setFloat("voxelScale", 1.0f/(voxelSize*dimension));
	prog->setFloat("voxelSize", voxelSize);
	prog->setInt("volumeDimension", dimension);
	prog->setVec3("worldMinPoint", sceneBoundingBox.MinPoint);

	auto workGroups = static_cast<unsigned int>(ceil(dimension / 8));
	glDispatchCompute(workGroups, workGroups, workGroups);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VoxelizationRenderer::RadiancePropagation()
{
	auto& prog = AssetsManager::Instance()->programs["injectPropagation"];
	prog->Use();
	//��radiance���ػ���
	glBindImageTexture(0, voxelRadiance, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	//��debug����
	glBindImageTexture(1, debug_comp, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	//��albedo��������
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, albedo);
	//�󶨷��߲�������
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, normal);
	//������mipmap����
	for (int i = 0; i < voxelAnisoMipmap.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE4+i);
		glBindTexture(GL_TEXTURE_3D, voxelAnisoMipmap[i]);
	}

	prog->setInt("volumeDimension", dimension);

	auto workGroups = static_cast<unsigned int>(ceil(dimension / 8));
	glDispatchCompute(workGroups, workGroups, workGroups);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VoxelizationRenderer::CalculateSceneBondingBox()
{
	auto model = AssetsManager::Instance()->models;
	sceneBoundingBox = AssetsManager::Instance()->models.begin()->second->boundingBox;

	//����boundingBox
	for (auto& model : AssetsManager::Instance()->models) {
		//������������͵�λ���سߴ�
		auto& boundingBox = model.second->boundingBox;
		sceneBoundingBox.MinPoint.x = glm::min(boundingBox.MinPoint.x, sceneBoundingBox.MinPoint.x);
		sceneBoundingBox.MinPoint.y = glm::min(boundingBox.MinPoint.y, sceneBoundingBox.MinPoint.y);
		sceneBoundingBox.MinPoint.z = glm::max(boundingBox.MinPoint.z, sceneBoundingBox.MinPoint.z);

		sceneBoundingBox.MaxPoint.x = glm::max(boundingBox.MaxPoint.x, sceneBoundingBox.MaxPoint.x);
		sceneBoundingBox.MaxPoint.y = glm::max(boundingBox.MaxPoint.y, sceneBoundingBox.MaxPoint.y);
		sceneBoundingBox.MaxPoint.z = glm::min(boundingBox.MaxPoint.z, sceneBoundingBox.MaxPoint.z);
	}

	//�������������
	sceneBoundingBox.Size = sceneBoundingBox.MaxPoint - sceneBoundingBox.MinPoint;
	sceneBoundingBox.Center = sceneBoundingBox.MinPoint + sceneBoundingBox.Size * 0.5f;
	gridSize = glm::max(sceneBoundingBox.Size.x, glm::max(sceneBoundingBox.Size.y, sceneBoundingBox.Size.z));
	voxelSize = gridSize / dimension;
}

void VoxelizationRenderer::DrawVoxel(VoxelMapType voxelMapType)
{
	//��ղ�������
	static GLfloat zero[] = { 0, 0, 0, 0 };
	glClearTexImage(debug_comp, 0, GL_RGBA, GL_FLOAT, zero);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//����������ɫ
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//�����ɫ�������Ȼ���

	//������ȾĿ��Ϊ����
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glViewport(0, 0, width, height);

	auto& progDrawVoxel = AssetsManager::Instance()->programs["DrawVoxel"];
	progDrawVoxel->Use();

	//��debug����
	glBindImageTexture(1, debug_comp, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	SetMVP_freeMove(progDrawVoxel);

	int mipLevel = 2;
	progDrawVoxel->setInt("mipLevel", mipLevel);

	glActiveTexture(GL_TEXTURE0);
	switch (voxelMapType)
	{
	case RADIANCE:
		glBindTexture(GL_TEXTURE_3D, voxelRadiance);
		break;
	case MIPMAP:
		progDrawVoxel->setInt("dimension", dimension / (mipLevel + 1));
		glBindTexture(GL_TEXTURE_3D, voxelAnisoMipmap[3]);
		break;
	case ALBEDO:
		glBindTexture(GL_TEXTURE_3D, albedo);
		break;
	}

	//�߹���,ֻ��Ϊ�˴��䶥����������ʵȫ�ɼ�����ɫ������
	glBindVertexArray(VAO_drawVoxel);
	glDrawArrays(GL_POINTS, 0, (dimension / (mipLevel + 1)) * (dimension / (mipLevel + 1)) * (dimension / (mipLevel + 1)));
	glBindVertexArray(0);
}

void VoxelizationRenderer::GenerateVoxelData()
{
	//����ǰ���������
	glViewport(0, 0, dimension, dimension);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//����������ɫ
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//�����ɫ�������Ȼ���
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//�ر�ͨ��д��
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//ʹ�����ػ���ɫ������
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];
	prog->Use();

	//��ղ�������
	static GLfloat zero[] = { 0, 0, 0, 0 };
	glClearTexImage(albedo, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(normal, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(emission, 0, GL_RGBA, GL_FLOAT, zero);
	for (int i = 0; i < 6; i++) {
		glClearTexImage(voxelAnisoMipmap[i], 0, GL_RGBA, GL_FLOAT, zero);
	}
	glClearTexImage(voxelRadiance, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(debug_comp, 0, GL_RGBA, GL_FLOAT, zero);

	//�����ػ�����
	glBindImageTexture(0, albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(2, emission, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

	//����MVP���԰�Χ��ĳ����ΪͶӰ���������ͶӰ
	SetMVP_ortho(prog, sceneBoundingBox);

	//����ģ��
	for (auto& model : AssetsManager::Instance()->models) {
		setModelMat(prog, model.second);
		for (auto& mesh : model.second->getMeshList()) {
			//������������
			if (mesh->material->diffuseMap != nullptr){
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mesh->material->diffuseMap->ID);
				prog->setBool("hasAlbedoMap", true);
			}

			//�󶨷�������
			if (mesh->material->normalMap != nullptr) {
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, mesh->material->normalMap->ID);
				prog->setBool("hasNormalMap", true);
			}

			//���Է�������
			if (mesh->material->emissionMap != nullptr) {
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, mesh->material->emissionMap->ID);
				prog->setBool("hasEmissionMap", true);
			}

			prog->setVec3("emission", mesh->material->Ke);
			prog->setVec3("albedo", mesh->material->Kd);

			mesh->Draw();
		}
	}
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VoxelizationRenderer::setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model)
{
	//����model����
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, model->position);
	modelM = modelM * glm::scale(modelM, model->scale);
	prog->setMat4("matrices.model", modelM);
	prog->setMat4("matrices.normal",glm::inverse(glm::transpose(modelM)));
}
