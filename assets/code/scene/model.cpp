#include <stdafx.h>
#include "model.h"


Model::Model(string const& path, bool gamma) : gammaCorrection(gamma)
{
	loadModel(path);
}

void Model::DrawBoundingBox()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

Model::~Model()
{
}

void Model::loadModel(string const& path)
{
	Assimp::Importer importer;
	scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	loadMeshes();//加载网格
	SetupBoundingBox();//设置包围盒

	scene = nullptr;
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

		//遍历获取顶点
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vector;

			//获取位置
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			//获取法线
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;

			//获取纹理坐标
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
				//获取切线
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;

				//获取副切线
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			//该顶点载入完毕
			vertices.push_back(vertex);

			//该顶点是否更新包围盒顶点
			boundingBox.MinPoint = min(boundingBox.MinPoint, vertex.Position);
			boundingBox.MaxPoint = max(boundingBox.MaxPoint, vertex.Position);
		}

		//获取所有图元（三角形）的索引
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		//加载材质
		material->loadMaterial(directory + "/" + materialName + ".pbr");

		materials.push_back(material);

		//该网格载入完毕
		meshes.push_back(make_shared<Mesh>(vertices, indices, material));
	}
}



void Model::SetupBoundingBox()
{
	//计算包围盒的中点和尺寸
	boundingBox.Center = (boundingBox.MinPoint + boundingBox.MaxPoint) * 0.5f;
	boundingBox.Size = boundingBox.MaxPoint- boundingBox.MinPoint;

	//设置VAO
	GLfloat vertices[] =
	{
		boundingBox.MinPoint.x, boundingBox.MinPoint.y, boundingBox.MinPoint.z,//左后下0
		boundingBox.MinPoint.x, boundingBox.MinPoint.y, boundingBox.MaxPoint.z,//左前下1
		boundingBox.MaxPoint.x, boundingBox.MinPoint.y, boundingBox.MaxPoint.z,//右前下2
		boundingBox.MaxPoint.x, boundingBox.MinPoint.y, boundingBox.MinPoint.z,//右后下3

		boundingBox.MinPoint.x, boundingBox.MaxPoint.y, boundingBox.MinPoint.z,//左后上4
		boundingBox.MinPoint.x, boundingBox.MaxPoint.y, boundingBox.MaxPoint.z,//左前上5
		boundingBox.MaxPoint.x, boundingBox.MaxPoint.y, boundingBox.MaxPoint.z,//右前上6
		boundingBox.MaxPoint.x, boundingBox.MaxPoint.y, boundingBox.MinPoint.z //右后上7
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
