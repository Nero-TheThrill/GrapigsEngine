#pragma once
#include <functional>			// std::function
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
		void AddData(const char* datum, unsigned tag = ERROR_INDEX) noexcept;
		bool Combo() noexcept;
		bool Selectable() noexcept;
		[[nodiscard]] int GetSelectedIndex() const noexcept;
		[[nodiscard]] std::string GetSelectedString() const noexcept;
		[[nodiscard]] unsigned GetSelectedID() const noexcept;
	private:
		std::string m_label{};
		int m_selected = 0;
		int m_dataSize = 0;
		std::vector<const char*> m_data;
		std::vector<unsigned> m_id;
	};

	struct WindowInst;
	class Window
	{
	public:
		Window(const char* name, WindowInst* p_inst) noexcept;
		~Window() = default;
		virtual void Update() noexcept;
		virtual void Content() noexcept = 0;
		virtual void SetObject(Object* p_object) noexcept;
		bool m_open = true;
	protected:
		WindowInst* m_p_windows = nullptr;
		Object* m_p_object = nullptr;
		const std::string m_name;
	};

	class Transform final : public Window
	{
	public:
		Transform(const char* name, WindowInst* p_inst) noexcept;
		void Content() noexcept override;
		void SetObject(Object* p_object) noexcept override;
		bool DoesReset() noexcept;
	private:
		glm::vec3 m_pos{ 0 }, m_rot{ 0 }, m_scl{ 1 };
		float m_uscl = 1, m_prevScl = 1;
		bool m_reset = false;	
	};

	class GizmoTool final : public Window
	{
	public:
		enum class Operation { None, Translation, Rotation, Scaling };
		GizmoTool(const char* name, WindowInst* p_inst) noexcept;
		void Update() noexcept override;
		void Content() noexcept override;
		Operation m_operation = Operation::None;
	private:
		int m_selected;
		const Texture m_texture[4];
		const intptr_t m_texId[4];
	};

	class Scene final : public Window
	{
	public:
		Scene(const char* name, WindowInst* p_inst) noexcept;
		void SetObject(Object* p_object) noexcept override;
		void Update() noexcept override;
		void Content() noexcept override;
	private:
		void UpdateGizmo() noexcept;
		glm::mat4 m_model{ 1.f }, m_view{ 1.f }, m_proj{ 1.f };
		glm::mat4 m_delta{ 1.f };
	};

	class Mesh final : public Window
	{
	public:
		Mesh(const char* name, WindowInst* p_inst) noexcept;
		void Content() noexcept override;
		void RecursiveMesh(Object* p_object, ::Mesh* p_mesh) noexcept;
	};

	class Material final : public Window
	{
	public:
		Material(const char* name, WindowInst* p_inst) noexcept;
		void Content() noexcept override;
		void SetObject(Object* p_object) noexcept override;
		[[nodiscard]] ::Mesh* GetMesh() const noexcept;
	private:
		void DrawTexture(Texture* texture, const char* desc) noexcept;
		std::string m_notice;
		::Mesh* m_p_mesh = nullptr;
		::Texture* m_p_clicked = nullptr;
	};

	class Asset final : public Window
	{
	public:
		Asset(const char* name, WindowInst* p_inst) noexcept;
		void Content() noexcept override;
		void SetObject(Object* p_object) noexcept override;
		void AddModelData(const ::Model* p_model) noexcept;
		void AddTextureData(Texture* p_texture) noexcept;
	private:
		DropDown m_modelDD, m_textureDD;
		std::set<unsigned> m_modelTags, m_textureTags;
		::Texture* m_p_texture;
	};

	class TextureModal : public Window
	{
		friend class GUIWindow::Asset;
	public:
		TextureModal(const char* name, WindowInst* p_inst) noexcept;
		void ImportTexture(const std::filesystem::path& path) noexcept;
		void Update() noexcept override;
		void Content() noexcept override;
	private:
		void OpenTextureModal() noexcept;
		void ModalContents(std::function<void(::Texture*)> apply_func) noexcept;
		void ApplyTextureToMesh(Texture* p_texture) const noexcept;
		void ApplyTextureToObject(Texture* p_texture) const noexcept;

		bool m_isNewTexture, m_applyTexture;
		::Texture* m_p_texture;
		std::queue<std::filesystem::path> m_texturePaths;
		DropDown m_texTypeDropDown;
	};

	class TexturePreview
	{
	public:
		void Update();
		void AddTexture(::Texture* p_texture) noexcept;
	private:
		std::map<::Texture*, bool> m_textures;
	};

	class TestWindow : public Window
	{
	public:
		TestWindow(const char* name, WindowInst* p_inst) noexcept;
		void Content() noexcept override;
	};

	struct WindowInst
	{
		WindowInst(ResourceManager* p_resource) noexcept;
		void SetObject(Object* p_object) noexcept;
		void Update() noexcept;
		Transform m_transformWin;
		Material m_materialWin;
		Scene m_sceneWin;
		Mesh m_meshWin;
		TextureModal m_textureModal;
		Asset m_assetWin;
		GizmoTool m_gizmoToolWin;
		TexturePreview m_texturePreview;
		TestWindow m_testWin;
		ResourceManager* m_p_resource;
	};
}