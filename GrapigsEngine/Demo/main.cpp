/*
 *	Author		: Jina Hyun
 *	Date		: 09/05/22
 *	File Name	: main.cpp
 *	Desc		: main function
 */

#include "Application.h"
#include "Input.h"
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
	Object* obj = rscmgr.LoadFbxAndCreateObject("model/spider.fbx", 0);
	FBXNodePrinter::SetFileToShowInfo("model/PenguinBaseMesh.fbx");
	rscmgr.SetGUIObject(obj);
	obj->SetTexture(rscmgr.LoadTexture("texture/Penguin.png"));
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

		if(Input::DropAndDropDetected())
		{
			const auto paths = Input::GetDroppedPaths();
			for(const auto& p : paths)
			{
				std::string cmprstr = p.extension().string();
				if (cmprstr == ".fbx")
				{
					FBXNodePrinter::SetFileToShowInfo(p);
					rscmgr.DeleteObject(obj);
					obj = rscmgr.LoadFbxAndCreateObject(p.string().c_str(), 0);

					rscmgr.SetGUIObject(obj);
				}
				else if(cmprstr == ".png"|| cmprstr == ".jpg")
				{
					obj->SetTexture(rscmgr.LoadTexture(p.string().c_str()));
				}
			}
		}
		FBXNodePrinter::UpdateGUI();
		rscmgr.UpdateObjectGUI();

		lights.Update();
		rscmgr.DrawTriangles();

		application.EndUpdate();
	}
	//delete mesh;
	application.CleanUp();
	return 0;
}