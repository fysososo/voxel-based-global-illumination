#pragma once
#include <stdafx.h>
#include "../scene/camera.h"
#include "../scene/model.h"
#include "../program/program.h"
#include "../renderer/renderer.h"
#include <map>


class AssetsManager
{
public:
	//单例模式，懒汉，线程不安全
	static unique_ptr<AssetsManager>& Instance();

	//所有资源
	map<string, shared_ptr<Camera>> cameras;//相机
	map<string, shared_ptr<Model>> models;//模型
	map<string, shared_ptr<Program>> programs;//shader程序
	map<string, shared_ptr<Renderer>> renderers;//渲染器

	//单例模式：删除外部的构造函数
	AssetsManager(AssetsManager const& r) = delete;
	AssetsManager(AssetsManager const&& r) = delete;
	AssetsManager& operator=(AssetsManager const& r) = delete;
	
	static void Terminate();
	~AssetsManager();

private:
	AssetsManager();
};