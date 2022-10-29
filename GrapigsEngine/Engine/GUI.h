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
	class DropDown
	{
	public:
		DropDown() = delete;
		DropDown(const std::string& label) noexcept;
		void ClearData() noexcept;
		void AddData(const char* datum) noexcept;
		bool Combo() noexcept;
		[[nodiscard]] int GetSelectedIndex() const noexcept;
		[[nodiscard]] std::string GetSelectedString() const noexcept;
	private:
		std::string m_label{};
		int m_selected = 0;
		int m_dataSize = 0;
		std::vector<const char*> m_data;
	};

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
		void RecursiveMesh(Object* p_object, Mesh* p_mesh) noexcept;
	};
	struct MaterialWin final : Window
	{
		void Content([[maybe_unused]] Object* object) override;
		std::string m_notice;
		Mesh* p_mesh = nullptr;
	};
public:
	GUI() noexcept;
	void SetResourceManager(ResourceManager* resource) noexcept;
	void SetObject(Object* object) noexcept;
	void Update() noexcept;
	Mesh* GetMesh() const noexcept;
	void ImportTexture(const std::filesystem::path& path) noexcept;

private:
	void DockSpace() noexcept;
	void MainMenuBar() noexcept;
	void ImportTextureModalUpdate() noexcept;
	ResourceManager* m_p_resourceManager = nullptr;
	Object* m_p_object = nullptr;
	TransformWin m_transformWin;
	SceneWin m_sceneWin;
	MeshWin m_meshWin;
	MaterialWin m_materialWin;
	std::filesystem::path m_texturePath;
	DropDown m_texTypeDropDown;
};