/*
 *	Author		: Jina Hyun
 *	Date		: 09/05/22
 *	File Name	: main.cpp
 *	Desc		: main function
 */	

#include "Application.h"
#include <imgui.h>
#include "ResourceManager.h"

int main(void)
{
	Application application;
	ResourceManager resource_manager;

	application.Init();
	
	Application::SetBackgroundColor(180, 210, 200);
	resource_manager.CompileShader(0, "../shader/test.vert", "../shader/test.frag");
	

	resource_manager.CreateObject(0, "cube", 0, 0);


	while(application.ShouldQuit() == false)
	{
		application.BeginUpdate();

		///////////////////////////////////////////////////////////
		/*
		 *	draw/rendering functions
		 */
		//ImGui::ShowDemoWindow();
		///////////////////////////////////////////////////////////
		application.EndUpdate();
	}
	application.CleanUp();
	return 0;
}