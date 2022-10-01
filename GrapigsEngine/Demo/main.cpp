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
	ResourceManager rscmgr;

	const std::vector<std::pair<ShaderType, std::filesystem::path>> shader_files = {
			std::make_pair(ShaderType::Vertex, "shader/test.vert"),
			std::make_pair(ShaderType::Fragment, "shader/test.frag")
	};

	application.Init();
	Application::SetBackgroundColor(180, 210, 200);

	rscmgr.CreateShader(shader_files);
	rscmgr.LoadFbxAndCreateObject("model/PenguinBaseMesh.fbx", 0);

	Lights lights;
	Light l1,l2;
	l1.m_transform.Translate(glm::vec3(0, -5, 5));
	l1.m_type = LightType::DIRECTIONAL;
	l2.m_transform.Translate(glm::vec3(0, -5, 5));
	l2.m_type = LightType::POINT;
	lights.AddLight(l1);
	lights.AddLight(l2);
	lights.Init();



	while(application.ShouldQuit() == false)
	{
		application.BeginUpdate();
		lights.Update();
		rscmgr.DrawTriangles();
		//ImGui::ShowDemoWindow();
		application.EndUpdate();
	}
	//delete mesh;
	application.CleanUp();
	return 0;
}