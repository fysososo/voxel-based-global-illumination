#include <stdafx.h>
#include "engine.h"

int main()
{
	try 
	{
		//这里是主循环
		Engine::Instance()->RenderLoop();
	}
	catch(int error)
	{
		switch (error)
		{
		case 0:
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return -1;
		case 1:
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}
	}

	Engine::Terminate();
	glfwTerminate();
	return 0;
}