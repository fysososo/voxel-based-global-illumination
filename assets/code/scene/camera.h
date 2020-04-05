#pragma once
#include <stdafx.h>
#include "../support/single_active.h"

//相机能做的运动：前后左右
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

//默认的一些相机参数
const float YAW = 0.0f;//偏航角
const float PITCH = 0.0f;//俯仰角
const float SPEED = 10.0f;//相机移动速度
const float SENSITIVITY = 0.1f;//相机转向敏感度
const float ZOOM = 45.0f;//

class Camera : public SingleActive<Camera>
{
public:
	glm::vec3 Position;//相机位置
	glm::vec3 Front;//观察空间的-z轴朝向
	glm::vec3 Up;//观察空间的y轴朝向
	glm::vec3 Right;//观察空间的x轴朝向
	glm::vec3 WorldUp;//世界空间的y轴朝向
	
	float Yaw;//偏航角
	float Pitch;//俯仰角
	float MovementSpeed;//相机移动速度
	float MouseSensitivity;//相机转向敏感度
	float Zoom;//缩放比例

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	//返回观察矩阵
	glm::mat4 GetViewMatrix();

	//处理方向键事件，更新相机的位置值
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	//处理鼠标移动事件，更新相机的偏航角和俯仰角
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	//处理鼠标滚轮事件，更新缩放比例
	void ProcessMouseScroll(float yoffset);

private:
	//通过偏航角和俯仰角计算相机方向
	void updateCameraVectors();
};
