#pragma once
#include <stdafx.h>

class Engine
{
public:
	//单例模式，懒汉，线程不安全
	static std::unique_ptr<Engine>& Instance();

	//单例模式：删除外部的构造函数
	Engine(Engine const& r) = delete;
	Engine(Engine const&& r) = delete;
	Engine& operator=(Engine const& r) = delete;
	
	//获取窗口指针
	GLFWwindow* Window();

	//渲染循环
	void RenderLoop();

	//终止程序
	static void Terminate();
	~Engine();

private:
	static GLboolean firstMouse;//是否第一次移动鼠标
	static GLfloat lastX;//鼠标的横坐标
	static GLfloat lastY;//鼠标的纵坐标
	static GLfloat deltaTime;//两帧间隔时间
	static GLfloat lastFrame;//上一帧的时间


	Engine();
	GLFWwindow* window;//GLFW窗口指针

	//初始化
	void Initialize();

	//设置回调函数
	void SetCallback();

	//一系列回调函数
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);//窗口大小变化回调函数
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);//鼠标移动回调函数
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//鼠标滚轮回调函数
	static void processInput(GLFWwindow* window);//键盘输入处理函数
};

