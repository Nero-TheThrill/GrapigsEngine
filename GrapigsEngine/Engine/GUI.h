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
	struct Window
	{
		void Update(Object* object);
		virtual void Content([[maybe_unused]] Object* object) = 0;
		bool m_open = true;
	};
	struct TransformWin final : Window
	{
		void Content([[maybe_unused]] Object* object) override;
		glm::vec3 m_pos{ 0 }, m_rot{ 0 }, m_scl{ 1 };
		float m_uscl = 1, m_prevScl = 1;
	};
	struct SceneWin final : Window { void Content([[maybe_unused]] Object* object) override; };
	struct MeshWin final : Window
	{
		void Content([[maybe_unused]] Object* object) override;
		void RecursiveMesh(Object* object, Mesh* mesh) noexcept;
	};
	struct MaterialWin final : Window
	{
		void Content([[maybe_unused]] Object* object) override;
		std::string m_notice;
		Mesh* p_mesh;
	};

public:
	void SetObject(Object* object) noexcept;
	void Update() noexcept;
private:
	void MainMenuBar();
	Object* p_object = nullptr;
	TransformWin transformWin;
	SceneWin sceneWin;
	MeshWin meshWin;
	MaterialWin materialWin;

	void ShowFullScreen() noexcept;
};