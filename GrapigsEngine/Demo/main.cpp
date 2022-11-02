/*
 *	Author		: Jina Hyun, Jinwoo Choi
 *	Date		: 09/05/22
 *	File Name	: main.cpp
 *	Desc		: main function
 */

#include "Application.h"
#include "Camera.h"
#include "Input.h"
#include "ResourceManager.h"
#include "GUI.h"

Lights *CreateLights()
{
	Lights* lights = new Lights();
	Light l1, l2;
	l1.m_transform.Translate(glm::vec3(-2, 2, 1));
	l1.m_type = LightType::DIRECTIONAL;
	l2.m_transform.Translate(glm::vec3(-0.7, 1.5, 0.7));
	l2.m_type = LightType::POINT;
	lights->AddLight(l1);
	lights->AddLight(l2);
	return lights;
}

void DragAndDrop(ResourceManager* r, GUI* g, Object* o)
{
	const auto paths = Input::GetDroppedPaths();
	for (const auto& p : paths)
	{
		std::string cmprstr = p.extension().string();
		if (cmprstr == ".fbx" || cmprstr == ".FBX")
		{
			o = r->CreateObject(p.string().c_str());
			g->SetObject(o);
		}
		else if (cmprstr == ".png" || cmprstr == ".jpg" || cmprstr == ".bmp")
		{
			g->ImportTexture(p);
		}
	}
}

Object* CreateObject(ResourceManager* r)
{
	const std::vector<std::pair<ShaderType, std::filesystem::path>> shader_files = {
			std::make_pair(ShaderType::Vertex, "shader/test.vert"),
			std::make_pair(ShaderType::Fragment, "shader/test.frag")
	};

	unsigned shaderTag = r->LoadShaders(shader_files);
	unsigned modelTag = r->LoadFbx("model/PenguinBaseMesh.fbx");
	unsigned textureTag = r->LoadTexture("texture/Penguin.png");
	return r->CreateObject(modelTag, shaderTag, textureTag);
}

int main(void)
{
	Application application(1200, 900);
	Application::SetBackgroundColor(255, 255, 255);
	ResourceManager* resource = new ResourceManager();
	GUI gui;
	gui.SetResourceManager(resource);
	Lights* lights = CreateLights();

	Object* obj = CreateObject(resource);
	gui.SetObject(obj);
	
	while(application.ShouldQuit() == false)
	{
		application.BeginUpdate();

		if(Input::DropAndDropDetected())
		{
			DragAndDrop(resource, &gui, obj);
		}
		gui.Update();
		lights->Update();
		resource->DrawTriangles();

		application.EndUpdate();
	}
	//delete mesh;
	application.CleanUp();
	delete lights;
	delete resource;
	return 0;
}