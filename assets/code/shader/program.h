#pragma once
#include <stdafx.h>

class Material;

//shader������Ļ���
class Program
{
public:
	//���๹�캯��������һ��Shader����
	Program();

	//��ȡ����ID
	GLuint getID();

	//����һ��Shader
	void AttachShader(GLenum shaderType, const std::string& filepath);

	// ���ӱ��������ӵ�����Shader������Shader����
	void Link();

	//ʹ�ø�Shader����
	void Use() const;

	//����ɫ��������һ�±�����һЩ�ӿ�
	#pragma region ���������������
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setUnsignedInt(const std::string& name, unsigned int value) const
	{
		glUniform1ui(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	#pragma endregion
	
	#pragma region ��������
	void setVec2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	
	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	
	void setVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	#pragma endregion

	#pragma region �������
	void setMat2(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	
	void setMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	
	void setMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	#pragma endregion

	~Program();
	Program& operator=(Program const& r) = delete;

protected:
	GLuint ID;//��shader�����ID

private:
	std::vector<GLuint> shaders;//������е�ID

	//���shader���������ʱ�Ĵ�����Ϣ
	void CheckErrors(GLuint shader, GLenum shaderType);

};

