/*
 *	Author		: Jina Hyun
 *	Date		: 10/18/22
 *	File Name	: GUI.cpp
 *	Desc		: ImGui contents
 */
#include "GUI.h"
#include <imgui.h>  // ImGui::

void GUI::SetObject(Object* object) noexcept
{
	m_obj = object;
	const Transform& t = m_obj->m_transform;
	m_pos = t.GetPosition();
	m_rot = t.GetRotation();
	m_scl = t.GetScaling();
	m_prevScl = 1;
	m_uscl = 1;
}

void GUI::Update() noexcept
{
    static bool open = true;
    ShowTransformWindow(&open);
    ShowSceneWindow(&open);
}

void GUI::ShowTransformWindow(bool* p_open) noexcept
{
	if(ImGui::Begin("Transform", p_open))
	{
        if (ImGui::DragFloat3("Translation", &m_pos[0], 0.01f, 0, 0, "%.2f"))
            m_obj->m_transform.Translate(m_pos);
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
            m_obj->m_transform.Rotate(m_rot.x, m_rot.y, m_rot.z);
        }
        if (ImGui::DragFloat3("Scaling", &m_scl[0], 0.01f, 0, 0, "%.2f"))
        {
            if (m_scl.x < std::numeric_limits<float>::epsilon())
                m_scl.x = 0.001f;
            if (m_scl.y < std::numeric_limits<float>::epsilon())
                m_scl.y = 0.001f;
            if (m_scl.z < std::numeric_limits<float>::epsilon())
                m_scl.z = 0.001f;
            m_obj->m_transform.Scale(m_scl);
            m_uscl = 1;
            m_prevScl = 1;
        }
        if (ImGui::DragFloat("Scale by", &m_uscl, 0.01f, 0, 0, "%.2f"))
        {
            if (m_uscl < std::numeric_limits<float>::epsilon())
                m_uscl = 0.001f;

            m_scl *= (m_uscl / m_prevScl);
            m_obj->m_transform.Scale(m_scl * m_scl);
            m_prevScl = m_uscl;
        }

	}
	ImGui::End();
}

void GUI::ShowSceneWindow(bool* p_open) noexcept
{
    if (ImGui::Begin("Scene", p_open))
    {
    	ImGui::Image((void*)ResourceManager::m_fbo->GetTexture(), ImGui::GetWindowSize(), ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
}
