#include <stdafx.h>
#include "engine.h"

GLboolean Engine::firstMouse = true;
GLfloat Engine::lastX = 0.0f;
GLfloat Engine::lastY = 0.0f;
GLfloat Engine::deltaTime = 0.0f;
GLfloat Engine::lastFrame = 0.0f;
GLint Engine::currentProcessLightIndex = 0;

std::unique_ptr<Engine>& Engine::Instance()
{
	static std::unique_ptr<Engine> instance = nullptr;

	if (!instance)
	{
		instance.reset(new Engine());
	}

	return instance;
}

Engine::Engine()
{
	//初始化
	Initialize();
		
	//载入所有资源
	AssetsManager::Instance();
	//设置回调函数
	SetCallback();
}

GLFWwindow* Engine::Window()
{
	if (window != nullptr)
	{
		return window;
	}
}

void Engine::Initialize()
{
	//初始化GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//OpenGL主版本
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);//OpenGL副版本
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//核心模式，不需要向后兼容立即渲染模式
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

	//创建GLFW窗口
	window = glfwCreateWindow(800, 600, "TestOpenGL", NULL, NULL);
	if (window == NULL)
	{
		throw 0;
	}
	glfwMakeContextCurrent(window);//将新创建的窗口的上下文设置为当前线程的的主上下文
	
	//获取屏幕位置
	GLint width, height;
	glfwGetWindowSize(window, &width, &height);
	lastX = width / 2;
	lastY = height / 2;


	//初始化GLAD
	//给GLAD传入(用来加载(系统相关的OpenGL函数指针地址)的)函数。
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw 1;
	}

	currentProcessLightIndex = 0;
}

void Engine::SetCallback()
{
	//注册窗口大小变化回调函数
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//设置鼠标移动回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//鼠标居中隐藏

	//设置鼠标滚轮回调函数
	glfwSetScrollCallback(window, scroll_callback);
}

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//OpenGL坐标（-1，1）将根据视口设置，映射到屏幕坐标
	glViewport(0, 0, width, height);//相对于窗口左下角的坐标、宽、高
}

void Engine::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 

	lastX = xpos;
	lastY = ypos;

	Camera::Active()->ProcessMouseMovement(xoffset, yoffset);
}

void Engine::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera::Active()->ProcessMouseScroll(yoffset);
}

bool Engine::processInput(GLFWwindow* window)
{
	bool renderStateChange = false;
	//每个按键都注册成回调函数太麻烦，不如一个个手动检查，统一在一个函数也更直观
	//按键状态在检查之前会保持（避免在检查前按键已经释放的问题）
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	auto& camera = Camera::Active();

	//处理相机的方向键
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		currentProcessLightIndex++;
		if (currentProcessLightIndex >= AssetsManager::Instance()->pointLights.size()) {
			currentProcessLightIndex = 0;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		AssetsManager::Instance()->pointLights[currentProcessLightIndex]->position.z += 2.0f;
		renderStateChange = true;
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		AssetsManager::Instance()->pointLights[currentProcessLightIndex]->position.z -= 2.0f;
		renderStateChange = true;
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		AssetsManager::Instance()->pointLights[currentProcessLightIndex]->position.x += 2.0f;
		renderStateChange = true;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		AssetsManager::Instance()->pointLights[currentProcessLightIndex]->position.x -= 2.0f;
		renderStateChange = true;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		auto& progLight = AssetsManager::Instance()->programs["lightPass"];
		progLight->Use();
		progLight->setInt("showMode", 0);
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		auto& progLight = AssetsManager::Instance()->programs["lightPass"];
		progLight->Use();
		progLight->setInt("showMode", 1);
	}	
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		auto& progLight = AssetsManager::Instance()->programs["lightPass"];
		progLight->Use();
		progLight->setInt("showMode", 2);
	}	
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		auto& progLight = AssetsManager::Instance()->programs["lightPass"];
		progLight->Use();
		progLight->setInt("showMode", 3);
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		auto& progLight = AssetsManager::Instance()->programs["lightPass"];
		progLight->Use();
		progLight->setInt("showMode", 4);
	}

	return renderStateChange;
}

void Engine::RenderLoop()
{
	AssetsManager::Instance()->renderers["DefferLight"]->SetMaterialUniforms();
	AssetsManager::Instance()->renderers["Voxelization"]->SetMaterialUniforms();
	AssetsManager::Instance()->renderers["Voxelization"]->Render();

	while (!glfwWindowShouldClose(window))
	{
		//计算每帧的间隔时间
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//处理键盘输入
		if (processInput(window)) {
			AssetsManager::Instance()->renderers["Voxelization"]->Render();
		}
		//AssetsManager::Instance()->renderers["Voxelization"]->Render();
		AssetsManager::Instance()->renderers["DefferLight"]->Render();
		
		glfwSwapBuffers(window);//交换颜色缓冲
		glfwPollEvents();//检查是否触发事件，并调用已注册的对应的回调函数

	}
}

void Engine::Terminate()
{
	delete Instance().release();
}

Engine::~Engine()
{
	AssetsManager::Terminate();
}

