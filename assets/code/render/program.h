#pragma once

#include <glad/glad.h>; 

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

/// <summary>
/// shader程序类的基类
/// </summary>
class Program
{
public:
	/// <summary>
	/// 基类构造函数：创建一个Shader程序
	/// </summary>
	Program();

	/// <summary>
	/// 附加一个Shader
	/// </summary>
	/// <param name="type">Shader类型</param>
	/// <param name="filepath">Shader源文件路径</param>
	void AttachShader(GLenum shaderType, const std::string& filepath);

	/// <summary>
	/// 链接编译所附加的所有Shader，生成Shader程序
	/// </summary>
	void Link();

	/// <summary>
	/// 使用该Shader程序
	/// </summary>
	void Use() const;

	~Program();
	Program& operator=(Program const& r) = delete;

protected:
	/// <summary>
	/// The program ID
	/// </summary>
	GLuint ID;

private:
	/// <summary>
	/// The Shaders ID
	/// </summary>
	std::vector<GLuint> shaders;

	/// <summary>
	/// 输出shader编译或链接时的错误信息
	/// </summary>
	/// <param name="shader">shader ID</param>
	/// <param name="shaderType">Shader类型</param>
	void CheckErrors(GLuint shader, GLenum shaderType);

};

