/*
 *	Author		: Jina Hyun
 *	Date		: 10/18/22
 *	File Name	: GUI.cpp
 *	Desc		: ImGui contents
 */
#include "GUI.h"
#include <string>
#include <imgui.h>  // ImGui::

#include "Input.h"

namespace
{
    template<typename T>
    std::string str(T value)
    {
        return std::to_string(value);
    }

    float ClampTo360(float a)
    {
        if (a > 360)
            return 0;
    	if (a < 0)
            return 360;
        return a;
    }
}

/* namespace - end ------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* GUI::DropDown - start ------------------------------------------------------------------------*/

GUI::DropDown::DropDown(const std::string& label) noexcept
    : m_label(label)
{
}

void GUI::DropDown::ClearData() noexcept
{
    m_data.clear();
    m_dataSize = 0;
    m_selected = 0;
}

void GUI::DropDown::AddData(const char* datum) noexcept
{
    m_dataSize++;
    m_data.push_back(datum);
}

bool GUI::DropDown::Combo() noexcept
{
    return ImGui::Combo(m_label.c_str(), &m_selected, m_data.data(), m_dataSize);
}

int GUI::DropDown::GetSelectedIndex() const noexcept
{
    return m_selected;
}

std::string GUI::DropDown::GetSelectedString() const noexcept
{
    return m_data[m_selected];
}

/* GUI::DropDown - end --------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* GUI - start ----------------------------------------------------------------------------------*/

void GUI::Window::Update(Object* object)
{
    if(m_open)
    {
        Content(object);
    }
}

/* Transform Window -----------------------------------------------------------------------------*/

void GUI::TransformWin::Content(Object* object)
{
    if (ImGui::Begin("Transform", &m_open))
    {
        if (ImGui::DragFloat3("Translation", &m_pos[0], 0.01f, 0, 0, "%.2f"))
            object->m_transform.Translate(m_pos);
        if (ImGui::DragFloat3("Rotation", &m_rot[0], 1, 0, 0, "%.1f"))
        {
            m_rot.x = ClampTo360(m_rot.x); m_rot.y = ClampTo360(m_rot.y); m_rot.z = ClampTo360(m_rot.z);
            object->m_transform.Rotate(m_rot.x, m_rot.y, m_rot.z);
        }
        if (ImGui::DragFloat3("Scaling", &m_scl[0], 0.01f, 0.001f, 1000, "%.2f"))
        {
            object->m_transform.Scale(m_scl);
            m_uscl = 1;
            m_prevScl = 1;
        }
        if (ImGui::DragFloat("Scale by", &m_uscl, 0.01f, 0, 0, "%.2f"))
        {
            if (m_uscl < std::numeric_limits<float>::epsilon())
                m_uscl = 0.001f;

            m_scl *= (m_uscl / m_prevScl);
            object->m_transform.Scale(m_scl * m_scl);
            m_prevScl = m_uscl;
        }

    }
    ImGui::End();
}

/* Scene Window ---------------------------------------------------------------------------------*/

void GUI::SceneWin::Content(Object* object)
{
    if (ImGui::Begin("Scene", &m_open))
    {
        const auto width = static_cast<float>(Input::s_m_windowSize.x);
        const auto height = static_cast<float>(Input::s_m_windowSize.y);
        const auto window = ImGui::GetContentRegionAvail();
        const float x = ((width - window.x) * 0.5f) / width;
        const float y = ((height - window.y) * 0.5f) / height;

        ImGui::Image((void*)ResourceManager::m_fbo->GetTexture(), window, ImVec2(x, 1 - y), ImVec2(1 - x, y));
    }
    ImGui::End();
}

/* Mesh Window ----------------------------------------------------------------------------------*/

void GUI::MeshWin::Content(Object* object)
{
    if (ImGui::Begin("Meshes", &m_open))
    {
        RecursiveMesh(object, &object->m_p_mesh->m_meshes[object->m_p_mesh->m_root]);
    }
    ImGui::End();
}

void GUI::MeshWin::RecursiveMesh(Object* p_object, Mesh* p_mesh) noexcept
{
    static int selection_mask = (1 << 2);
    static ImGuiTreeNodeFlags base = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
    static ImGuiTreeNodeFlags leaf = base | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    int clicked_index = -1;
    const bool is_selected = (selection_mask & (1 << p_mesh->index)) != 0;
    const bool is_leaf = p_mesh->children.empty();

    ImGuiTreeNodeFlags flag = is_leaf ? leaf : base;
    if (is_selected)
        flag |= ImGuiTreeNodeFlags_Selected;

    const bool is_open = ImGui::TreeNodeEx((void*)(intptr_t)p_mesh->index, flag, p_mesh->name.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        clicked_index = p_mesh->index;

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("_MeshNode", (const void*)&p_mesh->index, sizeof(int));
        ImGui::Text(p_mesh->name.c_str());
        ImGui::EndDragDropSource();
    }

    if (is_leaf == false && is_open == true)
    {
        for (const auto& c : p_mesh->children)
            RecursiveMesh(p_object, &p_object->m_p_mesh->m_meshes[c]);
        ImGui::TreePop();
    }

    if (clicked_index != -1) // Multi select
    {
        if (ImGui::GetIO().KeyCtrl)
            selection_mask ^= (1 << clicked_index);
        else
            selection_mask = (1 << clicked_index);
    }
}

/* Material Window ------------------------------------------------------------------------------*/

void GUI::MaterialWin::Content(Object* object)
{
    if (ImGui::Begin("Material", &m_open))
    {
        ImGui::Selectable(m_notice.c_str());
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_MeshNode"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                const int index = *(const int*)payload->Data;
                p_mesh = &object->m_p_mesh->m_meshes[index];
                m_notice = p_mesh->name;
            }
            ImGui::EndDragDropTarget();
        }

        if (p_mesh)
        {
            ImGui::Separator();
            Material& m = p_mesh->material;
            ImGui::SliderFloat("Metallic", &m.metallic, 0, 1, "%.2f");
            ImGui::SliderFloat("Roughness", &m.roughness, 0, 1, "%.2f");
        }
    }
    ImGui::End();
}

GUI::GUI() noexcept
    : m_texTypeDropDown("Texture Type")
{
    m_texTypeDropDown.AddData("Albedo(default)");
    m_texTypeDropDown.AddData("Metalness");
    m_texTypeDropDown.AddData("Roughness");
    m_texTypeDropDown.AddData("Ambient Occlusion");
    m_texTypeDropDown.AddData("Normal Map");
}

void GUI::SetResourceManager(ResourceManager* resource) noexcept
{
    m_p_resourceManager = resource;
}

void GUI::SetObject(Object* object) noexcept
{
    p_object = object;
	const Transform& t = p_object->m_transform;
    m_transformWin.m_pos = t.GetPosition();
    m_transformWin.m_rot = t.GetRotation();
    m_transformWin.m_scl = t.GetScaling();
    m_transformWin.m_prevScl = 1;
    m_transformWin.m_uscl = 1;
    m_materialWin.m_notice = "<Drag and Drop Mesh from Mesh Window>";
}

void GUI::Update() noexcept
{
    DockSpace();
    m_transformWin.Update(p_object);
    m_materialWin.Update(p_object);
    m_sceneWin.Update(p_object);
    m_meshWin.Update(p_object);
    ImportTextureModalUpdate();
}

Mesh* GUI::GetMesh() const noexcept
{
    return m_materialWin.p_mesh;
}

void GUI::ImportTexture(const std::filesystem::path& path) noexcept
{
    m_texturePath = path;
    if(m_materialWin.p_mesh != nullptr)
		ImGui::OpenPopup("Import Texture");
    else
		ImGui::OpenPopup("Import Texture Error");
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

void GUI::ImportTextureModalUpdate() noexcept
{
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();

    // Open texture import modal window
    // Select the type to import the texture
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Import Texture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(m_texturePath.filename().string().c_str());
        m_texTypeDropDown.Combo();
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            const auto tex = m_p_resourceManager->LoadTexture(m_texturePath.string().c_str());
            switch (m_texTypeDropDown.GetSelectedIndex())
            {
            case 1:
                m_materialWin.p_mesh->material.t_metallic = m_p_resourceManager->GetTexture(tex);
                break;
            case 2:
                m_materialWin.p_mesh->material.t_roughness = m_p_resourceManager->GetTexture(tex);
                break;
            case 3:
                m_materialWin.p_mesh->material.t_ao = m_p_resourceManager->GetTexture(tex);
                break;
            case 4:
                m_materialWin.p_mesh->material.t_normal = m_p_resourceManager->GetTexture(tex);
                break;
            default:
                m_materialWin.p_mesh->material.t_albedo = m_p_resourceManager->GetTexture(tex);
                break;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // If the mesh is not set, the texture won't be applied
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if(ImGui::BeginPopupModal("Import Texture Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("The mesh must be selected\n\ton the material window!");

        ImGui::SetCursorPosX(40);
        if (ImGui::Button("Okay", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

/* GUI - end ------------------------------------------------------------------------------------*/