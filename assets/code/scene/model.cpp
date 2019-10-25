#include <stdafx.h>
#include "model.h"


Model::Model(string const& path, bool gamma) : gammaCorrection(gamma)
{
	loadModel(path);
}

void Model::Draw()
{
	for (unsigned int i = 0; i < meshes.size(); i++)
	{
		meshes[i]->Draw();
	}
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

	loadMaterials();//加载材质
	loadMeshes();//加载网格
	SetupBoundingBox();//设置包围盒

	scene = nullptr;
}

void Model::loadMaterials()
{
	aiColor3D color;
	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* aMat = scene->mMaterials[i];
		shared_ptr<Material> material = make_shared<Material>();

		//获取材质名
		material->name = aMat->GetName().C_Str();//获取材质名

		//获取材质属性
		aMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material->Ka = glm::vec4(color.r, color.g, color.b, 1.0);//获取颜色albedo
		aMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->Kd = glm::vec4(color.r, color.g, color.b, 1.0);//获取漫反射diffuse
		aMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material->Ks = glm::vec4(color.r, color.g, color.b, 1.0);//获取镜反射specular

		//获取贴图
		material->diffuseMaps = loadMaterialTextures(aMat, aiTextureType_DIFFUSE, "texture_diffuse");//漫反射贴图
		material->specularMaps = loadMaterialTextures(aMat, aiTextureType_SPECULAR, "texture_specular");//镜面反射贴图
		material->normalMaps = loadMaterialTextures(aMat, aiTextureType_NORMALS, "texture_normal");//切线空间的法线贴图

		//该材质载入完毕
		materials.push_back(material);
	}
}

void Model::loadMeshes()
{
	for (GLuint i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		vector<Vertex> vertices;
		vector<GLuint> indices;
		shared_ptr<Material> material;

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

		//获取材质
		string matName = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
		for (GLint i = 0; i < materials.size(); i++)
		{
			if (materials[i]->name == matName)
			{
				material = materials[i];
				break;
			}
		}

		//该网格载入完毕
		meshes.push_back(make_shared<Mesh>(vertices, indices, material));
	}
}

vector<shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<shared_ptr<Texture>> MatTextures;//用于临时储存该材质该类型的所有贴图

	//遍历该材质的所有纹理
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		//获取该贴图的文件路径
		aiString str;
		mat->GetTexture(type, i, &str);

		// 检查该纹理是否已经加载（与已加载纹理比对文件路径）
		bool skip = false;
		for (GLuint j = 0; j < textures.size(); j++)
		{
			if (std::strcmp(textures[j]->Path.data(), str.C_Str()) == 0)
			{
				//已加载，只获取指针
				MatTextures.push_back(textures[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			//未加载，先创建该材质
			shared_ptr<Texture> texture = make_shared<Texture>();
			texture->ID = TextureFromFile(str.C_Str(), this->directory);
			texture->Type = typeName;
			texture->Path = str.C_Str();
			textures.push_back(texture);
			//再获取指针
			MatTextures.push_back(textures.back());
		}
	}
	return MatTextures;
}

GLuint Model::TextureFromFile(const GLchar* path, const string& directory, GLboolean gamma)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	GLuint textureID;
	glGenTextures(1, &textureID);

	GLint width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
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
