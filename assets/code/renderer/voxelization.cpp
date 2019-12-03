#include "stdafx.h"
#include "voxelization.h"

void VoxelizationRenderer::Render()
{
	SetAsActive();

	//����ǰ���������
	glViewport(0, 0, dimension, dimension);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//����������ɫ
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//�����ɫ�������Ȼ���
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//�ر�ͨ��д��
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//��ղ�������
	static GLfloat zero[] = { 0, 0, 0, 0 };
	glClearTexImage(albedo, 0, GL_RGBA, GL_FLOAT, zero);
	glBindImageTexture(0, albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

	//ʹ�����ػ���ɫ������
	auto& prog = AssetsManager::Instance()->programs["Voxelization"];
	prog->Use();
	prog->setFloat("voxelSize", voxelSize);
	prog->setVec3("boxMin", sceneBoundingBox.MinPoint);
	prog->setUnsignedInt("dimension", dimension);

	//����MVP���԰�Χ��ĳ����ΪͶӰ���������ͶӰ
	SetMVP_ortho(prog, sceneBoundingBox);

	//����ģ��
	for (auto& model : AssetsManager::Instance()->models) {
		setModelMat(prog, model.second);
		for (auto& mesh : model.second->meshes) {
			//������������
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(prog->getID(), "texture_diffuse"), 0);
			glBindTexture(GL_TEXTURE_2D, mesh->material->albedoMap);
			mesh->Draw();
		}
	}

	glBindTexture(GL_TEXTURE_3D, albedo);
	glGenerateTextureMipmap(albedo);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//����ͨ��д��
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
}

void VoxelizationRenderer::SetMaterialUniforms()
{
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
	gridSize = 0;//����ߴ磨=�Ӿ�����ߣ�
	dimension = 128;//һ�����ص�����
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

	//������������
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

	//ʹ�����ػ���ɫ������
	auto& prog = AssetsManager::Instance()->programs["DrawVoxel"];
	prog->Use();

	prog->setUnsignedInt("dimension", dimension);
	prog->setFloat("voxelSize", voxelSize);
	prog->setVec3("boxMin", sceneBoundingBox.MinPoint);

	//������
	glBindImageTexture(0, albedo, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);

	//����MVP����������ƶ�
	SetMVP_freeMove(prog);
	setModelMat(prog, model);

	//�߹���,ֻ��Ϊ�˴��䶥����������ʵȫ�ɼ�����ɫ������
	glBindVertexArray(VAO_drawVoxel);
	glDrawArrays(GL_POINTS, 0, dimension * dimension * dimension);
	glBindVertexArray(0);
}

void VoxelizationRenderer::setModelMat(shared_ptr<Program> prog, shared_ptr<Model> model)
{
	//����model����
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, model->position);
	modelM = glm::scale(modelM, glm::vec3(1.0f, 1.0f, 1.0f));
	prog->setMat4("model", modelM);
}

void VoxelizationRenderer::drawSceneBoundingBox()
{
	//����VAO
	GLfloat vertices[] =
	{
		sceneBoundingBox.MinPoint.x, sceneBoundingBox.MinPoint.y, sceneBoundingBox.MinPoint.z,//�����0
		sceneBoundingBox.MinPoint.x, sceneBoundingBox.MinPoint.y, sceneBoundingBox.MaxPoint.z,//��ǰ��1
		sceneBoundingBox.MaxPoint.x, sceneBoundingBox.MinPoint.y, sceneBoundingBox.MaxPoint.z,//��ǰ��2
		sceneBoundingBox.MaxPoint.x, sceneBoundingBox.MinPoint.y, sceneBoundingBox.MinPoint.z,//�Һ���3

		sceneBoundingBox.MinPoint.x, sceneBoundingBox.MaxPoint.y, sceneBoundingBox.MinPoint.z,//�����4
		sceneBoundingBox.MinPoint.x, sceneBoundingBox.MaxPoint.y, sceneBoundingBox.MaxPoint.z,//��ǰ��5
		sceneBoundingBox.MaxPoint.x, sceneBoundingBox.MaxPoint.y, sceneBoundingBox.MaxPoint.z,//��ǰ��6
		sceneBoundingBox.MaxPoint.x, sceneBoundingBox.MaxPoint.y, sceneBoundingBox.MinPoint.z //�Һ���7
	};

	GLuint indices[] =
	{
		0,1,2,//��1
		2,3,0,//��2
		4,5,7,//��1
		7,6,5,//��2
		1,2,5,//ǰ1
		5,6,2,//ǰ2
		0,3,7,//��1
		7,4,0,//��2
		0,1,5,//��1
		5,4,0,//��2
		2,3,7,//��1
		7,6,2 //��2
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
	//���ư�Χ��
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
