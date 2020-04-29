#pragma once
#include "mesh.h"

class Model
{
public:
	BoundingBox boundingBox;//��Χ��
	glm::vec3 position;
	glm::vec3 scale;

	//����ģ���ļ�·��������ģ��
	Model(string const& path, glm::vec3 position, glm::vec3 scale, bool gamma = false);
	vector<shared_ptr<Mesh>> getMeshList() { return meshes; }
	//����ģ��
	void Draw();

	//���ư�Χ��
	void DrawBoundingBox();

	~Model();

private:
	const aiScene* scene;//����ָ��
	string directory;//ģ���ļ�·��
	vector<shared_ptr<Texture>> textures;//��������
	vector<shared_ptr<Material>> materials;//���в��ʣ�����ָ�룩����
	vector<shared_ptr<Mesh>> meshes;//�������񣨶��㡢����������ָ�룩����
	bool gammaCorrection;//٤������
	GLuint VAO;//��Χ�е�VAO

	//����ģ��
	void loadModel(string const& path);

	//������ʣ�ͬʱ���ظ���������ͼ
	void loadMaterials();

	//��������
	void loadMeshes();

	//Ϊ���ʶ�ȡ�������ݣ�ȷ�����ظ����أ�
	shared_ptr<Texture> loadMaterialTexture(aiMaterial* mat, aiTextureType type, string typeName);

	//���ļ��л�ȡ��������
	GLuint TextureFromFile(const GLchar* path, const string& directory, GLboolean gamma = false);

	//���ð�Χ��
	void SetupBoundingBox();

};

