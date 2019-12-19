#include "stdafx.h"
#include "voxelization.h"

void VoxelizationRenderer::Render()
{
	SetAsActive();

	//绘制前的相关设置
	glViewport(0, 0, dimension, dimension);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置清屏颜色
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓存和深度缓存
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//关闭通道写入
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);


	//使用体素化着色器程序
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];
	prog->Use();
	prog->setFloat("voxelSize", voxelSize);
	prog->setVec3("boxMin", sceneBoundingBox.MinPoint);
	prog->setUnsignedInt("dimension", dimension);

	//清空并绑定纹理
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


	//绑定体素化对象
	glBindImageTexture(0, albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(1, normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(2, emission, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(3, roughness, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(4, metalness, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

	//设置MVP：以包围盒某个面为投影面进行正交投影
	SetMVP_ortho(prog, sceneBoundingBox);

	//绘制模型
	for (auto& model : AssetsManager::Instance()->models) {
		setModelMat(prog, model.second);
		for (auto& mesh : model.second->meshes) {
			//绑定漫反射纹理
			mesh->material->BindMap(prog, GL_TEXTURE0, Material::en_TEXTURE_ALBEDO);
			prog->setVec3("albedo", mesh->material->albedo);

			//绑定法线纹理
			mesh->material->BindMap(prog, GL_TEXTURE1, Material::en_TEXTURE_NORMAL);

			//绑定自发光纹理
			mesh->material->BindMap(prog, GL_TEXTURE2, Material::en_TEXTURE_EMISSION);
			prog->setVec3("emission", mesh->material->emission);

			//绑定粗糙度纹理
			mesh->material->BindMap(prog, GL_TEXTURE3, Material::en_TEXTURE_ROUGHNESS);
			prog->setFloat("roughness", mesh->material->roughness);

			//绑定金属度纹理
			mesh->material->BindMap(prog, GL_TEXTURE4, Material::en_TEXTURE_METANESS);
			prog->setFloat("metalness", mesh->material->metalness);

			mesh->Draw();
		}
	}
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	//开启通道写入
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	InjectRadiance();

	GenerateMipmapFirst(voxelRadiance);
	GenerateMipmapOthers();


}

void VoxelizationRenderer::SetMaterialUniforms()
{
	sceneBoundingBox = AssetsManager::Instance()->models.begin()->second->boundingBox;

	//计算boundingBox
	for (auto& model : AssetsManager::Instance()->models) {
		//计算体素网格和单位体素尺寸
		auto& boundingBox = model.second->boundingBox;
		sceneBoundingBox.MinPoint.x = glm::min(boundingBox.MinPoint.x, sceneBoundingBox.MinPoint.x);
		sceneBoundingBox.MinPoint.y = glm::min(boundingBox.MinPoint.y, sceneBoundingBox.MinPoint.y);
		sceneBoundingBox.MinPoint.z = glm::max(boundingBox.MinPoint.z, sceneBoundingBox.MinPoint.z);

		sceneBoundingBox.MaxPoint.x = glm::max(boundingBox.MaxPoint.x, sceneBoundingBox.MaxPoint.x);
		sceneBoundingBox.MaxPoint.y = glm::max(boundingBox.MaxPoint.y, sceneBoundingBox.MaxPoint.y);
		sceneBoundingBox.MaxPoint.z = glm::min(boundingBox.MaxPoint.z, sceneBoundingBox.MaxPoint.z);

		//gridSize = glm::max(boundingBox.Size.x, glm::max(boundingBox.Size.y, boundingBox.Size.z));
		//voxelSize = gridSize / dimension;
	}
	sceneBoundingBox.Size = sceneBoundingBox.MaxPoint - sceneBoundingBox.MinPoint;
	sceneBoundingBox.Center = (sceneBoundingBox.MaxPoint + sceneBoundingBox.MinPoint) * 0.5f;
	gridSize = glm::max(sceneBoundingBox.Size.x, glm::max(sceneBoundingBox.Size.y, sceneBoundingBox.Size.z));
	voxelSize = gridSize / dimension;

	//设置体素化程序参数
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];
	prog->Use();
	glUniform1i(glGetUniformLocation(prog->getID(), "AlbedoMap"), 0);
	glUniform1i(glGetUniformLocation(prog->getID(), "NormalMap"), 1);
	glUniform1i(glGetUniformLocation(prog->getID(), "EmissionMap"), 2);

	//设置逐体素直接光照程序参数
	auto& proginject = AssetsManager::Instance()->programs["injectRadiance"];
	proginject->Use();
	proginject->setFloat("F0", 0.04f);
}

VoxelizationRenderer::VoxelizationRenderer()
{
	gridSize = 0;//网格尺寸（=视景体最长边）
	dimension = 256;//一排体素的数量
	voxelSize = 0;//单位体素尺寸
	glGenVertexArrays(1, &VAO_drawVoxel);
	Set3DTexture();
}

void VoxelizationRenderer::SetMVP_freeMove(shared_ptr<Program> prog)
{
	//传递projection矩阵
	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glm::mat4 projectionM = glm::perspective(glm::radians(Camera::Active()->Zoom), (float)width / (float)height, 0.1f, 100.0f);
	prog->setMat4("projection", projectionM);

	//传递view矩阵
	glm::mat4 viewM = Camera::Active()->GetViewMatrix();
	prog->setMat4("view", viewM);
}

void VoxelizationRenderer::SetMVP_ortho(shared_ptr<Program> prog, BoundingBox& boundingBox)
{
	auto halfSize = gridSize / 2.0f;
	
	//projection矩阵
	glm::mat4 projectionM = glm::ortho
		(-halfSize, halfSize, -halfSize, halfSize, 0.0f, gridSize);

	//view矩阵
	glm::mat4 viewM[3];
	viewM[0] = glm::lookAt//看向yz平面
		(boundingBox.Center + glm::vec3(halfSize, 0.0f, 0.0f),
		 boundingBox.Center,
		 glm::vec3(0.0f, 1.0f, 0.0f));
	viewM[1] = glm::lookAt//看向xz平面
		(boundingBox.Center + glm::vec3(0.0f, halfSize, 0.0f),
		 boundingBox.Center,
		 glm::vec3(0.0f, 0.0f, -1.0f));
	viewM[2] = glm::lookAt//看向xy平面
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
	//创建albedo 3D纹理
	glGenTextures(1, &albedo);
	glBindTexture(GL_TEXTURE_3D, albedo);

	//设置纹理数据
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//创建normal 3D纹理
	glGenTextures(1, &normal);
	glBindTexture(GL_TEXTURE_3D, normal);

	//设置纹理数据
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//创建emission 3D纹理
	glGenTextures(1, &emission);
	glBindTexture(GL_TEXTURE_3D, emission);

	//设置纹理数据
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//创建roughness 3D纹理
	glGenTextures(1, &roughness);
	glBindTexture(GL_TEXTURE_3D, roughness);

	//设置纹理数据
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//创建roughness 3D纹理
	glGenTextures(1, &metalness);
	glBindTexture(GL_TEXTURE_3D, metalness);

	//设置纹理数据
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_FLOAT, nullptr);

	//创建roughness 3D纹理
	glGenTextures(1, &voxelRadiance);
	glBindTexture(GL_TEXTURE_3D, voxelRadiance);

	//设置纹理数据
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
		dimension, dimension, dimension,
		0, GL_RGBA, GL_FLOAT, nullptr);

	//创建6个方向的mipmap纹理
	for (int i = 0; i < 6; i++)
	{
		glGenTextures(1, &voxelAnisoMipmap[i]);
		glBindTexture(GL_TEXTURE_3D, voxelAnisoMipmap[i]);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
			dimension / 2, dimension / 2, dimension / 2,
			0, GL_RGBA, GL_FLOAT, nullptr);

		glGenerateMipmap(GL_TEXTURE_3D);
	}

}

void VoxelizationRenderer::GenerateMipmapOthers()
{
	//使用mipmapOthers计算着色器
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
			glActiveTexture(GL_TEXTURE0 + i);
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
	//使用mipmapFirst计算着色器
	auto& prog = AssetsManager::Instance()->programs["anisoMipmapFirst"];
	prog->Use();

	GLint halfDimension = dimension / 2;
	prog->setInt("mipDimension", halfDimension);

	//绑定六张纹理，用以接收第一级mipmap
	for (int i = 0; i < voxelAnisoMipmap.size(); ++i)
	{
		glBindImageTexture(i, voxelAnisoMipmap[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	}
	//绑定原始3D体素纹理
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, baseTexture);

	//开始计算，得到六张方向不同的第一级mipmap
	auto workGroups = static_cast<unsigned int>(ceil(halfDimension / 8));
	glDispatchCompute(workGroups, workGroups, workGroups);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VoxelizationRenderer::InjectRadiance()
{
	auto& prog = AssetsManager::Instance()->programs["injectRadiance"];
	prog->Use();

	//绑定Albedo体素
	glBindImageTexture(0, albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	//绑定Normal体素
	glBindImageTexture(1, normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	//绑定Emisson体素
	glBindImageTexture(2, emission, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	//绑定Roughness体素
	glBindImageTexture(3, roughness, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	//绑定Metalness体素
	glBindImageTexture(4, metalness, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
	//绑定输出纹理
	glBindImageTexture(5, voxelRadiance, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

	//设置光源
	prog->setInt("lightCount", (int)AssetsManager::Instance()->pointLights.size());
	for (int i = 0; i < AssetsManager::Instance()->pointLights.size(); i++) {
		auto& light = AssetsManager::Instance()->pointLights[i];
		prog->setVec3("pointLight[" + to_string(i) + "].position", light->position);
		prog->setVec3("pointLight[" + to_string(i) + "].color", light->color);
		prog->setFloat("pointLight[" + to_string(i) + "].intensity", light->intensity);
	}
	//设置摄像机位置
	prog->setVec3("viewPos", Camera::Active()->Position);

	//设置体素相关信息
	prog->setFloat("voxelScale", 1.0f/(voxelSize*dimension));
	prog->setFloat("voxelSize", voxelSize);
	prog->setInt("volumeDimension", dimension);
	prog->setVec3("worldMinPoint", sceneBoundingBox.MinPoint);

	auto workGroups = static_cast<unsigned int>(ceil(dimension / 8));
	glDispatchCompute(workGroups, workGroups, workGroups);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VoxelizationRenderer::setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model)
{
	//传递model矩阵
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, model->position);
	modelM = glm::scale(modelM, glm::vec3(1.0f, 1.0f, 1.0f));
	prog->setMat4("model", modelM);
}
