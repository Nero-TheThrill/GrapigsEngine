/*
 *	Author		: Jina Hyun
 *	Date		: 09/05/22
 *	File Name	: main.cpp
 *	Desc		: main function
 */	

#include "Application.h"
#include <imgui.h>

int main(void)
{
	Application application;
	application.Init();
	Application::SetBackgroundColor(180, 210, 200);
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