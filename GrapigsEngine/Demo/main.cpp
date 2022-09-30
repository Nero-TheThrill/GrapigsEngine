/*
 *	Author		: Jina Hyun
 *	Date		: 09/05/22
 *	File Name	: main.cpp
 *	Desc		: main function
 */	

#include "Application.h"

#include "ResourceManager.h"

int main(void)
{
	Application application;
	ResourceManager resource_manager;

	application.Init();
	resource_manager.LoadMesh("../test.fbx");
	Application::SetBackgroundColor(180, 210, 200);
	resource_manager.CompileShader(1, "../shader/test.vert", "../shader/test.frag");
	//Mesh* mesh = GetCubeMesh();
	//resource_manager.mesh_storage.push_back(mesh);
	Object* obj = resource_manager.CreateObject(0, "cube", 0, 1);

	obj->transform.Translate(glm::vec3(0, 0, -55));
	obj->transform.Scale(glm::vec3(0.2, 0.2, 0.2));

	while(application.ShouldQuit() == false)
	{
		application.BeginUpdate();

		obj->DrawTriangles();
		///////////////////////////////////////////////////////////
		/*
		 *	draw/rendering functions
		 */
		//ImGui::ShowDemoWindow();
		///////////////////////////////////////////////////////////
		application.EndUpdate();
	}
	//delete mesh;
	application.CleanUp();
	return 0;
}