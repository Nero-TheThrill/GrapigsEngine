/*
 *	Author		: Jina Hyun
 *	Date		: 09/05/22
 *	File Name	: main.cpp
 *	Desc		: main function
 */	

#include "Application.h"
#include <imgui.h>

#include "PlaceHolder_Cube.h"
#include "ResourceManager.h"

int main(void)
{
	Application application;
	ResourceManager resource_manager;

	application.Init();
	
	Application::SetBackgroundColor(180, 210, 200);
	resource_manager.CompileShader(1, "../shader/test.vert", "../shader/test.frag");
	Mesh* mesh = GetCubeMesh();
	resource_manager.mesh_storage.push_back(mesh);
	Object* obj = resource_manager.CreateObject(0, "cube", 1, 1);

	while(application.ShouldQuit() == false)
	{
		application.BeginUpdate();

		obj->Draw();
		///////////////////////////////////////////////////////////
		/*
		 *	draw/rendering functions
		 */
		//ImGui::ShowDemoWindow();
		///////////////////////////////////////////////////////////
		application.EndUpdate();
	}
	delete mesh;
	application.CleanUp();
	return 0;
}