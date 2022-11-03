/*
 *	Author		: Jina Hyun
 *	Date		: 10/18/22
 *	File Name	: GUI.h
 *	Desc		: ImGui contents
 */
#pragma once
#include "GUIWindow.h"	// GUIWindow::

class GUI
{
public:
	GUI(ResourceManager* p_resourceManager) noexcept;
	void SetObject(Object* object) noexcept;
	void Update() noexcept;

	[[nodiscard]] Mesh* GetMesh() const noexcept;
	void ImportTexture(const std::filesystem::path& path) noexcept;
private:
	void DockSpace() noexcept;
	void MainMenuBar() noexcept;
	Object* m_p_object;
	GUIWindow::WindowInst m_windows;
};