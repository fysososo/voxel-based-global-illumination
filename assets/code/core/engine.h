#pragma once
#include <stdafx.h>
#include "../renderer/DefferLightRenderer.h"

class Engine
{
public:
	//����ģʽ���������̲߳���ȫ
	static std::unique_ptr<Engine>& Instance();

	//����ģʽ��ɾ���ⲿ�Ĺ��캯��
	Engine(Engine const& r) = delete;
	Engine(Engine const&& r) = delete;
	Engine& operator=(Engine const& r) = delete;
	
	//��ȡ����ָ��
	GLFWwindow* Window();

	//��Ⱦѭ��
	void RenderLoop();

	//��ֹ����
	static void Terminate();
	~Engine();


private:
	static GLboolean firstMouse;//�Ƿ��һ���ƶ����
	static GLfloat lastX;//���ĺ�����
	static GLfloat lastY;//����������
	static GLfloat deltaTime;//��֡���ʱ��
	static GLfloat lastFrame;//��һ֡��ʱ��

	static GLint currentProcessLightIndex;

	Engine();
	GLFWwindow* window;//GLFW����ָ��

	//��ʼ��
	void Initialize();

	//���ûص�����
	void SetCallback();

	//һϵ�лص�����
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);//���ڴ�С�仯�ص�����
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);//����ƶ��ص�����
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//�����ֻص�����
	static bool processInput(GLFWwindow* window);//�������봦����
};

