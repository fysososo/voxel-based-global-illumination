#pragma once
class Renderer
{
public:
	/// <summary>
	/// 逐帧渲染循环里调用的统一接口
	/// </summary>
	virtual void Render() = 0;

	/// <summary>
	/// 给当前着色器程序传输材质信息（绘制mesh时调用）
	/// </summary>
	/// <param name="material">The material.</param>
	virtual void SetMaterialUniforms(const Material& material) const;
};

