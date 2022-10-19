/*
 *	Author		: Jina Hyun
 *	Date		: 10/18/22
 *	File Name	: GUI.h
 *	Desc		: ImGui contents
 */
#pragma once
#include "ResourceManager.h"

class GUI
{
public:
	void SetObject(Object* object) noexcept;
	void Update() noexcept;
private:
	void ShowTransformWindow(bool* p_open) noexcept;
	void ShowSceneWindow(bool* p_open) noexcept;
	Object* m_obj = nullptr;
	glm::vec3 m_pos{ 0 }, m_rot{ 0 }, m_scl{1};
	float m_uscl = 1, m_prevScl = 1;
};