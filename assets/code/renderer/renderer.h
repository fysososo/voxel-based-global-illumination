#pragma once

class Renderer : public SingleActive <Renderer>
{
public:
	//逐帧渲染循环里调用的统一接口
	virtual void Render() = 0;
	//给着色器程序传输材质信息（绘制mesh时调用）
	virtual void SetMaterialUniforms(Material& material);

};

