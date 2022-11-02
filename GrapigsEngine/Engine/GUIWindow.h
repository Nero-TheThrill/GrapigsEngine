#pragma once
#include <string>				// std::string
#include <queue>				// std::queue
#include <set>					// std::set
#include "ResourceManager.h"	// Object

namespace GUIWindow
{
	class DropDown
	{
		friend class GUI;
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
	
	class Window
	{
	public:
		Window(const char* name) noexcept;
		~Window() = default;
		virtual void Update() noexcept;
		virtual void Content() noexcept = 0;
		virtual void SetObject(Object* p_object) noexcept;
		bool m_open = true;
	protected:
		Object* m_p_object = nullptr;
		const std::string m_name;
	};

	class Transform final : public Window
	{
	public:
		Transform(const char* name) noexcept;
		void Content() noexcept override;
		void SetObject(Object* p_object) noexcept override;
	private:
		glm::vec3 m_pos{ 0 }, m_rot{ 0 }, m_scl{ 1 };
		float m_uscl = 1, m_prevScl = 1;
	};

	class Scene final : public Window
	{
	public:
		Scene(const char* name) noexcept;
		void Update() noexcept override;
		void Content() noexcept override;
	private:
		void UpdateGizmo() noexcept;
	};

	class Mesh final : public Window
	{
	public:
		Mesh(const char* name) noexcept;
		void Content() noexcept override;
		void RecursiveMesh(Object* p_object, ::Mesh* p_mesh) noexcept;
	};

	class Material final : public Window
	{
	public:
		Material(const char* name) noexcept;
		void Content() noexcept override;
		void SetObject(Object* p_object) noexcept override;
		[[nodiscard]] ::Mesh* GetMesh() const noexcept;
	private:
		std::string m_notice;
		::Mesh* p_mesh = nullptr;
	};

	class Asset final : public Window
	{
	public:
		Asset(const char* name) noexcept;
		void Content() noexcept override;
		void SetObject(Object* p_object) noexcept override;
		void AddModelData(const ::Model* p_model) noexcept;
		void AddTextureData(const Texture* p_texture) noexcept;
	private:
		DropDown m_modelDD, m_textureDD;
		std::set<unsigned> m_modelTags, m_textureTags;
	};

	class TextureModal
	{
	public:
		TextureModal() noexcept;
		void SetObject(Object* p_object) noexcept;
		void ImportTexture(const std::filesystem::path& path) noexcept;
		void Update(ResourceManager* p_resource, ::Mesh* p_mesh, Asset* asset_win) noexcept;
	private:
		void OpenTextureModal(const ::Mesh* p_mesh) const noexcept;
		bool m_open = false;
		Object* m_p_object = nullptr;
		std::queue<std::filesystem::path> m_texturePaths;
		DropDown m_texTypeDropDown;
	};
}