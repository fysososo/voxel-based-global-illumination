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

	//清空并绑定纹理
	static GLfloat zero[] = { 0, 0, 0, 0 };
	glClearTexImage(albedo, 0, GL_RGBA, GL_FLOAT, zero);
	glBindImageTexture(0, albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

	//使用体素化着色器程序
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];
	prog->Use();
	prog->setFloat("voxelSize", voxelSize);
	prog->setVec3("boxMin", sceneBoundingBox.MinPoint);
	prog->setUnsignedInt("dimension", dimension);

	//设置MVP：以包围盒某个面为投影面进行正交投影
	SetMVP_ortho(prog, sceneBoundingBox);

	//绘制模型
	for (auto& model : AssetsManager::Instance()->models) {
		setModelMat(prog, model.second);
		for (auto& mesh : model.second->meshes) {
			//绑定漫反射纹理
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(prog->getID(), "texture_diffuse"), 0);
			glBindTexture(GL_TEXTURE_2D, mesh->material->albedoMap);
			mesh->Draw();
		}
	}

	glBindTexture(GL_TEXTURE_3D, albedo);
	glGenerateTextureMipmap(albedo);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//开启通道写入
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
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
}

VoxelizationRenderer::VoxelizationRenderer()
{
	gridSize = 0;//网格尺寸（=视景体最长边）
	dimension = 128;//一排体素的数量
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
}


void VoxelizationRenderer::DrawVoxel(shared_ptr<Model> model)
{
	GLint width, height;
	glfwGetWindowSize(Engine::Instance()->Window(), &width, &height);
	glViewport(0, 0, width, height);

	//使用体素化着色器程序
	auto& prog = AssetsManager::Instance()->programs["DrawVoxel"];
	prog->Use();

	prog->setUnsignedInt("dimension", dimension);
	prog->setFloat("voxelSize", voxelSize);
	prog->setVec3("boxMin", sceneBoundingBox.MinPoint);

	//绑定纹理
	glBindImageTexture(0, albedo, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);

	//设置MVP：相机自由移动
	SetMVP_freeMove(prog);
	setModelMat(prog, model);

	//走过场,只是为了传输顶点索引，其实全由几何着色器绘制
	glBindVertexArray(VAO_drawVoxel);
	glDrawArrays(GL_POINTS, 0, dimension * dimension * dimension);
	glBindVertexArray(0);
}

void VoxelizationRenderer::setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model)
{
	//传递model矩阵
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, model->position);
	modelM = glm::scale(modelM, glm::vec3(1.0f, 1.0f, 1.0f));
	prog->setMat4("model", modelM);
}

void VoxelizationRenderer::drawSceneBoundingBox()
{
	//设置VAO
	GLfloat vertices[] =
	{
		sceneBoundingBox.MinPoint.x, sceneBoundingBox.MinPoint.y, sceneBoundingBox.MinPoint.z,//左后下0
		sceneBoundingBox.MinPoint.x, sceneBoundingBox.MinPoint.y, sceneBoundingBox.MaxPoint.z,//左前下1
		sceneBoundingBox.MaxPoint.x, sceneBoundingBox.MinPoint.y, sceneBoundingBox.MaxPoint.z,//右前下2
		sceneBoundingBox.MaxPoint.x, sceneBoundingBox.MinPoint.y, sceneBoundingBox.MinPoint.z,//右后下3

		sceneBoundingBox.MinPoint.x, sceneBoundingBox.MaxPoint.y, sceneBoundingBox.MinPoint.z,//左后上4
		sceneBoundingBox.MinPoint.x, sceneBoundingBox.MaxPoint.y, sceneBoundingBox.MaxPoint.z,//左前上5
		sceneBoundingBox.MaxPoint.x, sceneBoundingBox.MaxPoint.y, sceneBoundingBox.MaxPoint.z,//右前上6
		sceneBoundingBox.MaxPoint.x, sceneBoundingBox.MaxPoint.y, sceneBoundingBox.MinPoint.z //右后上7
	};

	GLuint indices[] =
	{
		0,1,2,//下1
		2,3,0,//下2
		4,5,7,//上1
		7,6,5,//上2
		1,2,5,//前1
		5,6,2,//前2
		0,3,7,//后1
		7,4,0,//后2
		0,1,5,//左1
		5,4,0,//左2
		2,3,7,//右1
		7,6,2 //右2
	};

	GLuint VAO, VBO, EBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//绘制包围盒
	glDisable(GL_CULL_FACE);
	auto& prog2 = AssetsManager::Instance()->programs["WhiteLine"];
	prog2->Use();
	SetMVP_freeMove(prog2);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
