#pragma once
#include<stdafx.h>
class Renderer : public SingleActive <Renderer>
{
public:
	//��֡��Ⱦѭ������õ�ͳһ�ӿ�
	virtual void Render() = 0;
	//����ɫ�������������Ϣ������meshʱ���ã�
	virtual void SetMaterialUniforms();

};

