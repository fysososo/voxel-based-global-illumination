#include <stdafx.h>
#include "model.h"


Model::Model(string const& path, glm::vec3 position, bool gamma) : gammaCorrection(gamma)
{
	loadModel(path, position);
}

void Model::DrawBoundingBox()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Model::Draw()
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i]->Draw();
	}
}

Model::~Model()
{
}

void Model::loadModel(string const& path, glm::vec3 position)
{
	cout << "��ʼ����ģ�ͣ�" << endl;
	cout << path << endl;
	cout << "���Եȡ���" << endl;
	this->position = position;
	Assimp::Importer importer;
	scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	loadMeshes();//��������
	SetupBoundingBox();//���ð�Χ��

	scene = nullptr;

	cout << "ģ�ͼ�����ɣ�" << endl;
}


void Model::loadMeshes()
{
	for (GLuint i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		vector<Vertex> vertices;
		vector<GLuint> indices;
		shared_ptr<Material> material = make_shared<Material>() ;
		string meshFileName = string(mesh->mName.C_Str());
		string meshName = meshFileName.substr(0, meshFileName.find_last_of('_'));
		string materialName = meshFileName.substr(meshName.length() + 1, meshFileName.length() - meshFileName.find_last_of('_') - 1);

		//������ȡ����
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;

			//��ȡλ��
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			//��ȡ����
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;

			//��ȡ��������
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}
			if (mesh->HasTangentsAndBitangents()) {
				//��ȡ����
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;

				//��ȡ������
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			//�ö����������
			vertices.push_back(vertex);

			//�ö����Ƿ���°�Χ�ж���
			boundingBox.MinPoint = min(boundingBox.MinPoint, vertex.Position);
			boundingBox.MaxPoint = max(boundingBox.MaxPoint, vertex.Position);
		}

		//��ȡ����ͼԪ�������Σ�������
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		//���ز���
		material->loadMaterial(directory + "/" + materialName + ".pbr");

		materials.push_back(material);

		//�������������
		meshes.push_back(make_shared<Mesh>(vertices, indices, material));
	}
}



void Model::SetupBoundingBox()
{
	//�����Χ�е��е�ͳߴ�
	boundingBox.MinPoint += position;
	boundingBox.MaxPoint += position;
	boundingBox.Center = (boundingBox.MinPoint + boundingBox.MaxPoint) * 0.5f;
	boundingBox.Size = boundingBox.MaxPoint- boundingBox.MinPoint;

	//����VAO
	GLfloat vertices[] =
	{
		boundingBox.MinPoint.x, boundingBox.MinPoint.y, boundingBox.MinPoint.z,//�����0
		boundingBox.MinPoint.x, boundingBox.MinPoint.y, boundingBox.MaxPoint.z,//��ǰ��1
		boundingBox.MaxPoint.x, boundingBox.MinPoint.y, boundingBox.MaxPoint.z,//��ǰ��2
		boundingBox.MaxPoint.x, boundingBox.MinPoint.y, boundingBox.MinPoint.z,//�Һ���3

		boundingBox.MinPoint.x, boundingBox.MaxPoint.y, boundingBox.MinPoint.z,//�����4
		boundingBox.MinPoint.x, boundingBox.MaxPoint.y, boundingBox.MaxPoint.z,//��ǰ��5
		boundingBox.MaxPoint.x, boundingBox.MaxPoint.y, boundingBox.MaxPoint.z,//��ǰ��6
		boundingBox.MaxPoint.x, boundingBox.MaxPoint.y, boundingBox.MinPoint.z //�Һ���7
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

	GLuint VBO, EBO;

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
}
