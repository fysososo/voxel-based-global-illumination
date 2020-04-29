#include <stdafx.h>
#include "model.h"


Model::Model(string const& path, glm::vec3 position, glm::vec3 scale, bool gamma) : gammaCorrection(gamma), position(position), scale(scale)
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

	loadMaterials();//���ز���
	loadMeshes();//��������
	SetupBoundingBox();//���ð�Χ��

	scene = nullptr;
}

void Model::loadMaterials()
{
	aiColor3D color;
	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* aMat = scene->mMaterials[i];
		shared_ptr<Material> material = make_shared<Material>();

		//��ȡ������
		material->name = aMat->GetName().C_Str();//��ȡ������

		//��ȡ��������
		aMat->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material->Ka = glm::vec4(color.r, color.g, color.b, 1.0);//��ȡ��ɫalbedo
		aMat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->Kd = glm::vec4(color.r, color.g, color.b, 1.0);//��ȡ������diffuse
		aMat->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material->Ks = glm::vec4(color.r, color.g, color.b, 1.0);//��ȡ������specular
		aMat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		material->Ke = glm::vec4(color.r, color.g, color.b, 1.0);//��ȡ�Է���emissive
		aMat->Get(AI_MATKEY_SHININESS, material->shiness);

		//��ȡ��ͼ
		material->diffuseMap = loadMaterialTexture(aMat, aiTextureType_DIFFUSE, "texture_diffuse");//��������ͼ
		material->specularMap = loadMaterialTexture(aMat, aiTextureType_SPECULAR, "texture_specular");//���淴����ͼ
		material->normalMap = loadMaterialTexture(aMat, aiTextureType_NORMALS, "texture_normal");//���߿ռ�ķ�����ͼ
		material->emissionMap = loadMaterialTexture(aMat, aiTextureType_EMISSION_COLOR, "texture_emission");//�Է�����ͼ

		//�ò����������
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

			//��ȡ����
			if (mesh->mTangents != NULL) {
				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;
			}
			else {
				vertex.Tangent = glm::normalize(glm::cross(vertex.Normal, glm::vec3(0.0f, 0.0f, 1.0f)));
			}

			if(mesh->mBitangents != NULL){
				//��ȡ������
				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			else {
				vertex.Bitangent = glm::normalize(glm::cross(vertex.Normal, vertex.Tangent));
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

		//��ȡ����
		string matName = scene->mMaterials[mesh->mMaterialIndex]->GetName().C_Str();
		for (GLint i = 0; i < materials.size(); i++)
		{
			if (materials[i]->name == matName)
			{
				material = materials[i];
				break;
			}
		}

		//�������������
		meshes.push_back(make_shared<Mesh>(vertices, indices, material));
	}
}

shared_ptr<Texture> Model::loadMaterialTexture(aiMaterial* mat, aiTextureType type, string typeName)
{
	shared_ptr<Texture> MatTextures;

	//��ȡ����ͼ���ļ�·��
	aiString str;
	mat->GetTexture(type, 0, &str);

	if (str.length == 0) {
		MatTextures = nullptr;
		return MatTextures;
	}

	// ���������Ƿ��Ѿ����أ����Ѽ�������ȶ��ļ�·����
	bool skip = false;
	for (GLuint j = 0; j < textures.size(); j++)
	{
		if (std::strcmp(textures[j]->Path.data(), str.C_Str()) == 0)
		{
			//�Ѽ��أ�ֻ��ȡָ��
			MatTextures = textures[j];
			skip = true;
			break;
		}
	}
	if (!skip)
	{
		//δ���أ��ȴ����ò���
		shared_ptr<Texture> texture = make_shared<Texture>();
		texture->ID = TextureFromFile(str.C_Str(), this->directory);
		texture->Type = typeName;
		texture->Path = str.C_Str();
		textures.push_back(texture);
		//�ٻ�ȡָ��
		MatTextures = textures.back();
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
	//�����Χ�е��е�ͳߴ�
	boundingBox.Center = (boundingBox.MinPoint + boundingBox.MaxPoint) * 0.5f;
	boundingBox.Size = boundingBox.MaxPoint - boundingBox.MinPoint;

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
