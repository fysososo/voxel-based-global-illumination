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

	////����ͨ��д��
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
}

void VoxelizationRenderer::SetMaterialUniforms()
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
	//������Χ������������չ
	//sceneBoundingBox.MinPoint -= (sceneBoundingBox.Size * 0.1f);
	//sceneBoundingBox.MaxPoint += (sceneBoundingBox.Size * 0.1f);

	//�������������
	sceneBoundingBox.Size = sceneBoundingBox.MaxPoint - sceneBoundingBox.MinPoint;
	sceneBoundingBox.Center = sceneBoundingBox.MinPoint + sceneBoundingBox.Size * 0.5f;
	gridSize = glm::max(sceneBoundingBox.Size.x, glm::max(sceneBoundingBox.Size.y, sceneBoundingBox.Size.z));
	voxelSize = gridSize / dimension;

	//�������ػ��������
	auto& progVoxelization = AssetsManager::Instance()->programs["Voxelization"];
	progVoxelization->Use();
	glUniform1i(glGetUniformLocation(progVoxelization->getID(), "AlbedoMap"), 0);
	glUniform1i(glGetUniformLocation(progVoxelization->getID(), "NormalMap"), 1);
	glUniform1i(glGetUniformLocation(progVoxelization->getID(), "EmissionMap"), 2);

	//����������ֱ�ӹ��ճ������
	auto& proginject = AssetsManager::Instance()->programs["injectRadiance"];
	proginject->Use();
	proginject->setFloat("F0", 0.04f);
	proginject->setFloat("traceShadowHit", 0.5f);

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
	prog->setFloat("voxelScale", 1.0f/(voxelSize*dimension));
	prog->setVec3("worldMinPoint", sceneBoundingBox.MinPoint);
	prog->setUnsignedInt("dimension", dimension);

	//��ղ�������
	static GLfloat zero[] = { 0, 0, 0, 0 };
	glClearTexImage(albedo, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(normal, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(emission, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(roughness, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(metalness, 0, GL_RGBA, GL_FLOAT, zero);
	for (int i = 0; i < 6; i++) {
		glClearTexImage(voxelAnisoMipmap[i], 0, GL_RGBA, GL_FLOAT, zero);
	}
	glClearTexImage(voxelRadiance, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(debug_comp, 0, GL_RGBA, GL_FLOAT, zero);
	glClearTexImage(debug_comp_injectRadiance, 0, GL_RGBA, GL_FLOAT, zero);


	//�����ػ�����
	glBindImageTexture(0, albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(2, emission, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(3, roughness, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(4, metalness, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

	//����MVP���԰�Χ��ĳ����ΪͶӰ���������ͶӰ
	SetMVP_ortho(prog, sceneBoundingBox);

	//����ģ��
	for (auto& model : AssetsManager::Instance()->models) {
		setModelMat(prog, model.second);
		for (auto& mesh : model.second->meshes) {
			//������������
			mesh->material->BindMap(prog, GL_TEXTURE0, Material::en_TEXTURE_ALBEDO);
			prog->setVec3("albedo", mesh->material->albedo);

			//�󶨷�������
			mesh->material->BindMap(prog, GL_TEXTURE1, Material::en_TEXTURE_NORMAL);

			//���Է�������
			mesh->material->BindMap(prog, GL_TEXTURE2, Material::en_TEXTURE_EMISSION);
			prog->setVec3("emission", mesh->material->emission);

			//�󶨴ֲڶ�����
			mesh->material->BindMap(prog, GL_TEXTURE3, Material::en_TEXTURE_ROUGHNESS);
			prog->setFloat("roughness", mesh->material->roughness);

			//�󶨽���������
			mesh->material->BindMap(prog, GL_TEXTURE4, Material::en_TEXTURE_METANESS);
			prog->setFloat("metalness", mesh->material->metalness);

			mesh->Draw();
		}
	}
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
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
	glm::mat4 projectionM = glm::perspective(glm::radians(Camera::Active()->Zoom), (float)width / (float)height, 0.1f, 100.0f);
	prog->setMat4("projection", projectionM);

	//����view����
	glm::mat4 viewM = Camera::Active()->GetViewMatrix();
	prog->setMat4("view", viewM);
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

	//����roughness 3D����
	glGenTextures(1, &roughness);
	glBindTexture(GL_TEXTURE_3D, roughness);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//����metalness 3D����
	glGenTextures(1, &metalness);
	glBindTexture(GL_TEXTURE_3D, metalness);
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

	//����debug 3D����
	glGenTextures(1, &debug_comp_injectRadiance);
	glBindTexture(GL_TEXTURE_3D, debug_comp_injectRadiance);
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
	glBindImageTexture(1, normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	//��Emisson����
	glBindImageTexture(2, emission, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
	//��Roughness����
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, roughness);
	//��Metalness����
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_3D, metalness);
	//���������
	glBindImageTexture(5, voxelRadiance, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glBindImageTexture(6, debug_comp_injectRadiance, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
	

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

void VoxelizationRenderer::setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model)
{
	//����model����
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, model->position);
	modelM = glm::scale(modelM, glm::vec3(1.0f, 1.0f, 1.0f));
	prog->setMat4("matrices.model", modelM);
	prog->setMat4("matrices.normal",glm::inverse(glm::transpose(modelM)));
}
