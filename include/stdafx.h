#pragma once
#include <windows.h>
//OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//数学
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//支持
#include "../assets/code/support/stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//C++
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <memory>
using namespace std;


//自定义结构
struct Texture
{
	GLuint ID;
	string Type;
	string Path;
};

struct Vertex
{
	glm::vec3 Position;//位置
	glm::vec3 Normal;//法线
	glm::vec2 TexCoords;//纹理坐标
	glm::vec3 Tangent;//切线
	glm::vec3 Bitangent;//副切线
};

struct BoundingBox {
	glm::vec3 MinPoint = glm::vec3(std::numeric_limits<float>::infinity());//坐标最小点
	glm::vec3 MaxPoint = glm::vec3(std::numeric_limits<float>::lowest());//坐标最大点
	glm::vec3 Center;//中心点
	glm::vec3 Size;//尺寸
};

//自定义文件
#include "../assets/code/core/assets.h"
#include "../assets/code/core/engine.h"
