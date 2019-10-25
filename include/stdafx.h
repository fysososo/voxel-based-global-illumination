#pragma once
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
struct Texture {
	GLuint id;
	string type;
	string path;
};

struct Vertex {
	glm::vec3 Position;//位置
	glm::vec3 Normal;//法线
	glm::vec2 TexCoords;//纹理坐标
	glm::vec3 Tangent;//切线
	glm::vec3 Bitangent;//副切线
};

//自定义文件
#include "../assets/code/core/assets.h"
#include "../assets/code/core/engine.h"
