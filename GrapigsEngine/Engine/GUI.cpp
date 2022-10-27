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
}

void GUI::Window::Update(Object* object)
{
    if(m_open)
    {
        Content(object);
    }
}

void GUI::TransformWin::Content(Object* object)
{
    if (ImGui::Begin("Transform", &m_open))
    {
        if (ImGui::DragFloat3("Translation", &m_pos[0], 0.01f, 0, 0, "%.2f"))
            object->m_transform.Translate(m_pos);
        if (ImGui::DragFloat3("Rotation", &m_rot[0], 1, 0, 0, "%.1f"))
        {
            if (m_rot.x > 360)
                m_rot.x = 0;
            else if (m_rot.x < 0)
                m_rot.x = 360;
            if (m_rot.y > 360)
                m_rot.y = 0;
            else if (m_rot.y < 0)
                m_rot.y = 360;
            if (m_rot.z > 360)
                m_rot.z = 0;
            else if (m_rot.z < 0)
                m_rot.z = 360;
            object->m_transform.Rotate(m_rot.x, m_rot.y, m_rot.z);
        }
        if (ImGui::DragFloat3("Scaling", &m_scl[0], 0.01f, 0, 0, "%.2f"))
        {
            if (m_scl.x < std::numeric_limits<float>::epsilon())
                m_scl.x = 0.001f;
            if (m_scl.y < std::numeric_limits<float>::epsilon())
                m_scl.y = 0.001f;
            if (m_scl.z < std::numeric_limits<float>::epsilon())
                m_scl.z = 0.001f;
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

void GUI::SceneWin::Content(Object* object)
{
    if (ImGui::Begin("Scene", &m_open))
    {
        const auto width = static_cast<float>(Input::s_m_windowSize.x);
        const auto height = static_cast<float>(Input::s_m_windowSize.y);
        const auto window = ImGui::GetWindowSize();
        const float x = ((width - window.x) * 0.5f) / width;
        const float y = ((height - window.y) * 0.5f) / height;
        ImGui::Image((void*)ResourceManager::m_fbo->GetTexture(), ImGui::GetWindowSize(), ImVec2(x, 1 - y), ImVec2(1 - x, y));
    }
    ImGui::End();
}

void GUI::MeshWin::Content(Object* object)
{
    if (ImGui::Begin("Meshes", &m_open))
    {
        RecursiveMesh(object, &object->m_p_mesh->m_meshes[object->m_p_mesh->m_root]);
    }
    ImGui::End();
}

void GUI::MeshWin::RecursiveMesh(Object* object, Mesh* mesh) noexcept
{
    static int selection_mask = (1 << 2);
    static ImGuiTreeNodeFlags base = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
    static ImGuiTreeNodeFlags leaf = base | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    int clicked_index = -1;
    const bool is_selected = (selection_mask & (1 << mesh->index)) != 0;
    const bool is_leaf = mesh->children.empty();

    ImGuiTreeNodeFlags flag = is_leaf ? leaf : base;
    if (is_selected)
        flag |= ImGuiTreeNodeFlags_Selected;

    const bool is_open = ImGui::TreeNodeEx((void*)(intptr_t)mesh->index, flag, mesh->name.c_str());
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        clicked_index = mesh->index;

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("_MeshNode", (const void*)&mesh->index, sizeof(int));
        ImGui::Text(mesh->name.c_str());
        ImGui::EndDragDropSource();
    }

    if (is_leaf == false && is_open == true)
    {
        for (const auto& c : mesh->children)
            RecursiveMesh(object, &object->m_p_mesh->m_meshes[c]);
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
            ImGui::SliderFloat("Ambient", &m.ambient, 0, 1, "%.2f");
            ImGui::SliderFloat("Diffuse", &m.diffuse, 0, 1, "%.2f");
            ImGui::SliderFloat("Specular", &m.specular, 0, 1, "%.2f");
        }
    }
    ImGui::End();
}

void GUI::SetObject(Object* object) noexcept
{
    p_object = object;
	const Transform& t = p_object->m_transform;
    transformWin.m_pos = t.GetPosition();
    transformWin.m_rot = t.GetRotation();
    transformWin.m_scl = t.GetScaling();
    transformWin.m_prevScl = 1;
    transformWin.m_uscl = 1;
    materialWin.m_notice = "<Drag and Drop Material>";
}

void GUI::Update() noexcept
{
    ShowFullScreen();
    MainMenuBar();
    transformWin.Update(p_object);
    materialWin.Update(p_object);
    sceneWin.Update(p_object);
    meshWin.Update(p_object);
}

Mesh* GUI::GetMesh() noexcept
{
    return materialWin.p_mesh;
}

void GUI::MainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if(ImGui::BeginMenu("Windows"))
        {
            ImGui::MenuItem("Transform Window", "", &transformWin.m_open);
            ImGui::MenuItem("Material Window", "", &materialWin.m_open);
            ImGui::MenuItem("Scene Window", "", &sceneWin.m_open);
            ImGui::MenuItem("Mesh Window", "", &meshWin.m_open);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void GUI::ShowFullScreen() noexcept
{
    static bool background = true;
    if (background)
    {
        static ImGuiWindowFlags flags = 
            ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoBackground
            | ImGuiWindowFlags_NoBringToFrontOnFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::Begin("Example: Fullscreen window", &background, flags);
        ImGui::End();
    }
}
