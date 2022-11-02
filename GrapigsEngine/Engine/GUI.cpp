/*
 *	Author		: Jina Hyun
 *	Date		: 10/18/22
 *	File Name	: GUI.cpp
 *	Desc		: ImGui contents
 */
#include "GUI.h"
#include <imgui.h>  // ImGui functions

/* GUI - start ----------------------------------------------------------------------------------*/

GUI::GUI(ResourceManager* p_resourceManager) noexcept
    : m_p_resourceManager(p_resourceManager),
	m_p_object(nullptr),
    m_transformWin("Transform"),
    m_materialWin("Material"),
    m_sceneWin("Scene"),
    m_meshWin("Mesh"),
    m_assetWin("Imported Asset")
{
}

void GUI::SetObject(Object* object) noexcept
{
    m_p_object = object;
    m_transformWin.SetObject(m_p_object);
    m_materialWin.SetObject(m_p_object);
    m_sceneWin.SetObject(m_p_object);
    m_meshWin.SetObject(m_p_object);
    m_textureModal.SetObject(m_p_object);
    m_assetWin.SetObject(m_p_object);
}

void GUI::Update() noexcept
{
    DockSpace();
    m_transformWin.Update();
    m_materialWin.Update();
    m_meshWin.Update();
    m_textureModal.Update(m_p_resourceManager, m_materialWin.GetMesh(), &m_assetWin);
    m_assetWin.Update();
    m_sceneWin.Update();
}

Mesh* GUI::GetMesh() const noexcept
{
    return m_materialWin.GetMesh();
}

void GUI::ImportTexture(const std::filesystem::path& path) noexcept
{
    m_textureModal.ImportTexture(path);
}

void GUI::DockSpace() noexcept
{
    constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    constexpr ImGuiWindowFlags window_flags =  ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    // Set docking window style
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Set position and size
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::Begin("DockSpace", NULL, window_flags);
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(ImGui::GetID("DockSpace"), ImVec2(0.0f, 0.0f), dockspace_flags);

	MainMenuBar();

    ImGui::End();
}

void GUI::MainMenuBar() noexcept
{
    if (ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("Transform Window", "", &m_transformWin.m_open);
            ImGui::MenuItem("Material Window", "", &m_materialWin.m_open);
            ImGui::MenuItem("Scene Window", "", &m_sceneWin.m_open);
            ImGui::MenuItem("Mesh Window", "", &m_meshWin.m_open);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}


/* GUI - end ------------------------------------------------------------------------------------*/