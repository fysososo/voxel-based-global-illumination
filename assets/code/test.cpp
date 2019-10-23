#include <stdafx.h>
#include "scene/model.h"
#include "render/prog_test.h"
#include "scene/camera.h"

//着色器源码：已使用文件单独储存
//#pragma region 顶点着色器源码
//const char* vertexShaderSource = "#version 330 core\n"
//"layout (location = 0) in vec3 aPos;\n"
//"layout (location = 1) in float aColor;\n"
//"layout (location = 2) in vec2 aTexCoord;\n"
//"out float outColor;\n"
//"out vec2 TexCoord;\n"
//"void main()\n"
//"{\n"
//"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
//"   outColor = aColor;\n"
//"   TexCoord = aTexCoord;\n"
//"}\0";
//#pragma endregion
//
//#pragma region 片元着色器源码
//const char* fragmentShaderSource = "#version 330 core\n"
//"#extension GL_ARB_explicit_uniform_location : enable\n"
//"out vec4 FragColor;\n"
//"in float outColor;\n"
//"in vec2 TexCoord;;\n"
//"layout (location = 0) uniform float blueValue;\n"
//"layout (location = 1) uniform sampler2D ourTexture;\n"
//"void main()\n"
//"{\n"
//"   FragColor =  mix(texture(ourTexture, TexCoord), vec4(1.0f * outColor, 0.5 * outColor, blueValue * outColor, 1.0f), 0.4);\n"
//"}\n\0";
//#pragma endregion

#pragma region 全局的相关设置
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
#pragma endregion

#pragma region 窗口大小变化回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//OpenGL坐标（-1，1）将根据视口设置，映射到屏幕坐标
	glViewport(0, 0, width, height);//相对于窗口左下角的坐标、宽、高
}
#pragma endregion

#pragma region 鼠标移动回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
#pragma endregion

#pragma region 鼠标滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
#pragma endregion

#pragma region 键盘输入处理函数
void processInput(GLFWwindow *window)
{
	//每个按键都注册成回调函数太麻烦，不如一个个手动检查，统一在一个函数也更直观
	//按键状态在检查之前会保持（避免在检查前按键已经释放的问题）
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//处理相机的方向键
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}
#pragma endregion



int main()
{
	
	#pragma region 初始化GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//OpenGL主版本
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);//OpenGL副版本
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//核心模式，不需要向后兼容立即渲染模式
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//Mac OS X系统需要
	#pragma endregion

	#pragma region 创建GLFW窗口
	//设置窗口宽、高、名称，创建窗口
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TestOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);//将新创建的窗口的上下文设置为当前线程的的主上下文
	#pragma endregion

	#pragma region 初始化GLAD
	//给GLAD传入(用来加载(系统相关的OpenGL函数指针地址)的)函数。
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	#pragma endregion

	#pragma region 注册回调函数
	//注册窗口大小变化回调函数：
	//窗口第一次显示或大小变化时，调用该函数来实时调整视口大小
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//设置鼠标移动回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//鼠标居中隐藏

	//设置鼠标滚轮回调函数
	glfwSetScrollCallback(window, scroll_callback);

	#pragma endregion

	#pragma region 测试封装好的着色器程序类
	ProgTest progTest = ProgTest();
	progTest.AttachShader(GL_VERTEX_SHADER, "assets/code/render/shader/test.vert");
	progTest.AttachShader(GL_FRAGMENT_SHADER, "assets/code/render/shader/test.frag");
	progTest.Link();

	#pragma region 已注释代码
	//#pragma region 设置顶点着色器
	//GLuint vertexShader;
	//vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	//glCompileShader(vertexShader);
	//GLint success;
	//GLchar infoLog[512];
	//glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);//获取最后一次编译是否成功的信息
	//if (!success)
	//{
	//	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);//获取信息日志
	//	std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	//}
	//#pragma endregion

	//#pragma region 设置片元着色器
	//GLuint fragmentShader;
	//fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	//glCompileShader(fragmentShader);
	//glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);//获取最后一次编译是否成功的信息
	//if (!success)
	//{
	//	glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);//获取着色器编译信息日志
	//	std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	//}
	//#pragma endregion

	//#pragma region 链接所有着色器生成着色器程序
	//GLuint shaderProgram;
	//shaderProgram = glCreateProgram();
	//glAttachShader(shaderProgram, vertexShader);
	//glAttachShader(shaderProgram, fragmentShader);
	//glLinkProgram(shaderProgram);
	//glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);//获取最后一次链接是否成功的信息
	//if (!success) {
	//	glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);//获取着色器程序链接信息日志
	//	std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
	//}
	////过河桥：只是标记为删除而已，没有附加在任何程序时才会真正删除
	//glDeleteShader(vertexShader);
	//glDeleteShader(fragmentShader);
	//#pragma endregion
	#pragma endregion

	#pragma endregion

	#pragma region 测试封装好的模型类

	Model testModel("assets/model/test/nanosuit.obj");

	#pragma region 已注释代码
	//#pragma region 把从哪些VBO，如何获取顶点，绑定了哪个EBO的信息储存到VAO中
	//GLuint VAO;
	//glGenVertexArrays(1, &VAO);
	//glBindVertexArray(VAO);

	//	#pragma region 把顶点坐标和纹理坐标传输到显存（VBO）中
	//	//三角形的三个顶点数据
	//	GLfloat vertices[] = {
	//		//     ---- 位置 ----     - 纹理坐标 -
	//			 0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // 右上
	//			 0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // 右下
	//			-0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // 左下
	//			-0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // 左上
	//	};
	//	GLuint VBO;
	//	glGenBuffers(1, &VBO);
	//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//	//一下子把所有顶点传到显存中，比一个点一个点绘制效率高
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//	#pragma endregion

	//	#pragma region 顶点着色器将从显存（VBO）中读入顶点坐标和纹理坐标
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	//	glEnableVertexAttribArray(0);//启用位置为0的顶点属性（默认禁用）
	//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//	glEnableVertexAttribArray(2);//启用位置为0的顶点属性（默认禁用）
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);//收工，但是显存的数据还在，只是暂时不操作它而已
	//	#pragma endregion

	//	#pragma region 不同属性可以有不同的数据来源，再开一个VBO，储存顶点颜色明暗度
	//	float inColor[] = {
	//		0.3f,0.6f,0.9f,0.8f
	//	};
	//	GLuint VBO2;
	//	glGenBuffers(1, &VBO2);
	//	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	//	glBufferData(GL_ARRAY_BUFFER, sizeof(inColor), inColor, GL_STATIC_DRAW);
	//	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	//	glEnableVertexAttribArray(1);//启用位置为0的顶点属性（默认禁用）
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);//收工，但是显存的数据还在，只是暂时不操作它而已
	//	#pragma endregion

	//	#pragma region 索引缓冲对象EBO
	//	GLuint indices[] = {
	//		0, 1, 3, // 第一个三角形
	//		1, 2, 3  // 第二个三角形
	//	};
	//	GLuint EBO;
	//	glGenBuffers(1, &EBO);
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	//	#pragma endregion
	
	//glBindVertexArray(0);//设置完毕，先收工，等需要绘制的时候再绑定
	//#pragma endregion

	//#pragma region 加载贴图，生成纹理
	//GLuint texture;
	//glGenTextures(1, &texture);
	//glBindTexture(GL_TEXTURE_2D, texture);
	////为当前绑定的纹理对象设置环绕、过滤方式
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	////载入图片
	//int width, height, nrChannels;
	//stbi_set_flip_vertically_on_load(true);//翻转Y轴
	//unsigned char* data = stbi_load("assets/img/picture.jpg", &width, &height, &nrChannels, 0);
	//if (data)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	//	glGenerateMipmap(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	std::cout << "Failed to load texture" << std::endl;
	//}
	//stbi_image_free(data);
	//#pragma endregion


	#pragma endregion

	#pragma endregion

	glEnable(GL_DEPTH_TEST);

	#pragma region 渲染循环
	while (!glfwWindowShouldClose(window))
	{
		//计算每帧的间隔时间
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);//处理键盘输入

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置清屏颜色
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//清空颜色缓存和深度缓存
		
		#pragma region 绘制
		progTest.Use();//使用设置好的着色器程序

		//传递projection矩阵
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		progTest.setMat4("projection", projection);

		//传递view矩阵
		glm::mat4 view = camera.GetViewMatrix();
		progTest.setMat4("view", view);
		
		//传递model矩阵
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
		progTest.setMat4("model", model);

		testModel.Draw(progTest);//绘制
		#pragma endregion

		glfwSwapBuffers(window);//交换颜色缓冲
		glfwPollEvents();//检查是否触发事件，并调用已注册的对应的回调函数

		
	}
	#pragma endregion
	
	glfwTerminate();//释放/删除之前分配的所有资源
	return 0;
}