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
	//��ʼ��
	Initialize();
		
	//����������Դ
	AssetsManager::Instance();
	//���ûص�����
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
	//��ʼ��GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//OpenGL���汾
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);//OpenGL���汾
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//����ģʽ������Ҫ������������Ⱦģʽ
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

	//����GLFW����
	window = glfwCreateWindow(800, 600, "TestOpenGL", NULL, NULL);
	if (window == NULL)
	{
		throw 0;
	}
	glfwMakeContextCurrent(window);//���´����Ĵ��ڵ�����������Ϊ��ǰ�̵߳ĵ���������
	
	//��ȡ��Ļλ��
	GLint width, height;
	glfwGetWindowSize(window, &width, &height);
	lastX = width / 2;
	lastY = height / 2;


	//��ʼ��GLAD
	//��GLAD����(��������(ϵͳ��ص�OpenGL����ָ���ַ)��)������
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw 1;
	}

	currentProcessLightIndex = 0;
}

void Engine::SetCallback()
{
	//ע�ᴰ�ڴ�С�仯�ص�����
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//��������ƶ��ص�����
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//����������

	//���������ֻص�����
	glfwSetScrollCallback(window, scroll_callback);
}

void Engine::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//OpenGL���꣨-1��1���������ӿ����ã�ӳ�䵽��Ļ����
	glViewport(0, 0, width, height);//����ڴ������½ǵ����ꡢ����
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
	//ÿ��������ע��ɻص�����̫�鷳������һ�����ֶ���飬ͳһ��һ������Ҳ��ֱ��
	//����״̬�ڼ��֮ǰ�ᱣ�֣������ڼ��ǰ�����Ѿ��ͷŵ����⣩
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	auto& camera = Camera::Active();

	//��������ķ����
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
		//����ÿ֡�ļ��ʱ��
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//�����������
		if (processInput(window)) {
			AssetsManager::Instance()->renderers["Voxelization"]->Render();
		}
		//AssetsManager::Instance()->renderers["Voxelization"]->Render();
		AssetsManager::Instance()->renderers["DefferLight"]->Render();
		
		glfwSwapBuffers(window);//������ɫ����
		glfwPollEvents();//����Ƿ񴥷��¼�����������ע��Ķ�Ӧ�Ļص�����

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

