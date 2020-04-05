#pragma once
#include <stdafx.h>
#include "../support/single_active.h"

//����������˶���ǰ������
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

//Ĭ�ϵ�һЩ�������
const float YAW = 0.0f;//ƫ����
const float PITCH = 0.0f;//������
const float SPEED = 10.0f;//����ƶ��ٶ�
const float SENSITIVITY = 0.1f;//���ת�����ж�
const float ZOOM = 45.0f;//

class Camera : public SingleActive<Camera>
{
public:
	glm::vec3 Position;//���λ��
	glm::vec3 Front;//�۲�ռ��-z�ᳯ��
	glm::vec3 Up;//�۲�ռ��y�ᳯ��
	glm::vec3 Right;//�۲�ռ��x�ᳯ��
	glm::vec3 WorldUp;//����ռ��y�ᳯ��
	
	float Yaw;//ƫ����
	float Pitch;//������
	float MovementSpeed;//����ƶ��ٶ�
	float MouseSensitivity;//���ת�����ж�
	float Zoom;//���ű���

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	//���ع۲����
	glm::mat4 GetViewMatrix();

	//��������¼������������λ��ֵ
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	//��������ƶ��¼������������ƫ���Ǻ͸�����
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	//�����������¼����������ű���
	void ProcessMouseScroll(float yoffset);

private:
	//ͨ��ƫ���Ǻ͸����Ǽ����������
	void updateCameraVectors();
};
