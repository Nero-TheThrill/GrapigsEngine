#include "GUIWindow.h"

#include <imgui.h>              // ImGui functions
#include "ImGuizmo/ImGuizmo.h"  // ImGuizmo

#include "Input.h"              // Input::s_windowSize
#include "Camera.h"             // CameraBuffer

namespace GUIWindow
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

	/*-----------------------------------------------------------------------------------------------*/
	/* DropDown - start -----------------------------------------------------------------------------*/

    DropDown::DropDown(const std::string& label) noexcept
        : m_label(label)
    {
    }

    void DropDown::ClearData() noexcept
    {
        m_data.clear();
        m_dataSize = 0;
        m_selected = 0;
    }

    void DropDown::AddData(const char* datum) noexcept
    {
        m_dataSize++;
        m_data.push_back(datum);
    }

    bool DropDown::Combo() noexcept
    {
        return ImGui::Combo(m_label.c_str(), &m_selected, m_data.data(), m_dataSize);
    }

    int DropDown::GetSelectedIndex() const noexcept
    {
        return m_selected;
    }

    std::string DropDown::GetSelectedString() const noexcept
    {
        return m_data[m_selected];
    }

	/* DropDown - end -------------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------------------------------*/
	/* GUI::Window - start --------------------------------------------------------------------------*/

    Window::Window(const char* name) noexcept
        : m_name(name)
    {
    }

    void Window::Update() noexcept
    {
        if(m_open)
        if(ImGui::Begin(m_name.c_str()), &m_open)
        {
            Content();
            ImGui::End();
        }
    }

    void Window::SetObject(Object* p_object) noexcept
    {
        m_p_object = p_object;
    }

    /* GUI::Window - end ----------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------------------------------*/
	/* Transform Window - start ---------------------------------------------------------------------*/

	Transform::Transform(const char* name) noexcept : Window(name) {    }

    void Transform::Content() noexcept
    {
        if (ImGui::DragFloat3("Translation", &m_pos[0], 0.01f, 0, 0, "%.2f"))
            m_p_object->m_transform.Translate(m_pos);
        if (ImGui::DragFloat3("Rotation", &m_rot[0], 1, 0, 0, "%.1f"))
        {
            m_rot.x = ClampTo360(m_rot.x); m_rot.y = ClampTo360(m_rot.y); m_rot.z = ClampTo360(m_rot.z);
            m_p_object->m_transform.Rotate(m_rot.x, m_rot.y, m_rot.z);
        }
        if (ImGui::DragFloat3("Scaling", &m_scl[0], 0.01f, 0.001f, 1000, "%.2f"))
        {
            m_p_object->m_transform.Scale(m_scl);
            m_uscl = 1;
            m_prevScl = 1;
        }
        if (ImGui::DragFloat("Scale by", &m_uscl, 0.01f, 0, 0, "%.2f"))
        {
            if (m_uscl < std::numeric_limits<float>::epsilon())
                m_uscl = 0.001f;

            m_scl *= (m_uscl / m_prevScl);
            m_p_object->m_transform.Scale(m_scl * m_scl);
            m_prevScl = m_uscl;
        }
    }

    void Transform::SetObject(Object* p_object) noexcept
    {
        Window::SetObject(p_object);
        const ::Transform& t = m_p_object->m_transform;
        m_pos = t.GetPosition();
        m_rot = t.GetRotation();
        m_scl = t.GetScaling();
        m_prevScl = 1;
        m_uscl = 1;
    }

    /* Transform Window - end -----------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Scene Window - start -------------------------------------------------------------------------*/

    Scene::Scene(const char* name) noexcept : Window(name) {    }

    void Scene::Update() noexcept
    {
        ImGuizmo::BeginFrame();
        if (ImGui::Begin(m_name.c_str(), &m_open))
        {
            Content();
            UpdateGizmo();
            ImGui::End();
        }
    }

    void Scene::Content() noexcept
    {
        const auto width = static_cast<float>(Input::s_m_windowSize.x);
        const auto height = static_cast<float>(Input::s_m_windowSize.y);
        const auto window = ImGui::GetContentRegionAvail();
        const float x = ((width - window.x) * 0.5f) / width;
        const float y = ((height - window.y) * 0.5f) / height;

        const auto texture = static_cast<intptr_t>(ResourceManager::m_fbo->GetTexture());
        ImGui::Image(reinterpret_cast<void*>(texture), window, ImVec2(x, 1 - y), ImVec2(1 - x, y));
    }

    void Scene::UpdateGizmo() noexcept
    {
        // Set viewport
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        const auto& size = ImGui::GetWindowSize();
        const auto& pos = ImGui::GetWindowPos();
        ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

        // Camera
        Camera* cam = CameraBuffer::GetMainCamera();
        glm::mat4 view = cam->GetWorldToCameraMatrix();
        glm::mat4 proj = cam->GetCameraToNDCMatrix();

        // Object transform
        glm::mat4 transform = m_p_object->m_transform.GetTransformMatrix();

        ImGuizmo::Manipulate(&view[0][0], &proj[0][0], ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, &transform[0][0]);
    }

    /* Scene Window - end ---------------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Mesh Window - start --------------------------------------------------------------------------*/

    Mesh::Mesh(const char* name) noexcept : Window(name) {    }

    void Mesh::Content() noexcept
    {
        const auto root = m_p_object->m_p_model->m_root;
        RecursiveMesh(m_p_object, &m_p_object->m_p_model->m_meshes[root]);
    }

    void Mesh::RecursiveMesh(Object* p_object, ::Mesh* p_mesh) noexcept
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
                RecursiveMesh(p_object, &p_object->m_p_model->m_meshes[c]);
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

    /* Mesh Window - end ----------------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Material Window - start ----------------------------------------------------------------------*/

    Material::Material(const char* name) noexcept : Window(name) {    }

    void Material::Content() noexcept
    {
        ImGui::Selectable(m_notice.c_str());
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_MeshNode"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                const int index = *(const int*)payload->Data;
                p_mesh = &m_p_object->m_p_model->m_meshes[index];
                m_notice = p_mesh->name;
            }
            ImGui::EndDragDropTarget();
        }

        if (p_mesh)
        {
            ImGui::Separator();
            ::Material& m = p_mesh->material;
            ImGui::SliderFloat("Metallic", &m.metallic, 0, 1, "%.2f");
            ImGui::SliderFloat("Roughness", &m.roughness, 0, 1, "%.2f");
        }
    }

    void Material::SetObject(Object* p_object) noexcept
    {
        Window::SetObject(p_object);
        m_notice = "<Drag and Drop Mesh from Mesh Window>";
    }

    ::Mesh* Material::GetMesh() const noexcept
    {
        return p_mesh;
    }

    /* Material Window - end ------------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Asset Window - start -------------------------------------------------------------------------*/

    Asset::Asset(const char* name) noexcept
        : Window(name), m_modelDD("Models"), m_textureDD("Textures")
    {
    }

    void Asset::Content() noexcept
    {
        m_modelDD.Combo();
        ImGui::Separator();
        m_textureDD.Combo();
    }

    void Asset::SetObject(Object* p_object) noexcept
    {
	    Window::SetObject(p_object);

        AddModelData(p_object->m_p_model);

        const auto& meshes = m_p_object->m_p_model->m_meshes;
        std::set<Texture*> textures;
        for (std::size_t i = 1; i < meshes.size(); ++i)
        {
            const auto& mat = meshes[i].material;
            if (mat.t_albedo)       textures.insert(mat.t_albedo);
            if (mat.t_ao)           textures.insert(mat.t_ao);
            if (mat.t_metallic)     textures.insert(mat.t_metallic);
            if (mat.t_normal)       textures.insert(mat.t_normal);
            if (mat.t_roughness)    textures.insert(mat.t_roughness);
        }
        for (auto& texture : textures)
            AddTextureData(texture);
    }

    void Asset::AddModelData(const ::Model* p_model) noexcept
    {
        for (const auto& tag : m_modelTags)
        {
            if (p_model->m_tag == tag)
                return;
        }
        m_modelTags.insert(p_model->m_tag);
        m_modelDD.AddData(p_model->m_name.c_str());
    }

    void Asset::AddTextureData(const Texture* p_texture) noexcept
    {
        for (const auto& tag : m_textureTags)
        {
            if (p_texture->m_tag == tag)
                return;
        }
        m_textureTags.insert(p_texture->m_tag);
        m_textureDD.AddData(p_texture->m_name.c_str());
    }

    /* Asset Window - end ---------------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Texture Modal Window - start -----------------------------------------------------------------*/

	TextureModal::TextureModal() noexcept
        : m_open(false), m_p_object(nullptr), m_texTypeDropDown("Texture Type")
    {
        m_open = false;
        m_texTypeDropDown.AddData("Albedo(default)");
        m_texTypeDropDown.AddData("Metalness");
        m_texTypeDropDown.AddData("Roughness");
        m_texTypeDropDown.AddData("Ambient Occlusion");
        m_texTypeDropDown.AddData("Normal Map");
    }

	void TextureModal::SetObject(Object* p_object) noexcept
	{
        m_p_object = p_object;
	}

	void TextureModal::ImportTexture(const std::filesystem::path& path) noexcept
    {
        m_texturePaths.push(path);
        m_open = true;
    }

    void TextureModal::OpenTextureModal(const ::Mesh* p_mesh) const noexcept
    {
        if (p_mesh != nullptr)
        {
            if (p_mesh->parent == -1) // root
                ImGui::OpenPopup("Import Texture To All Meshes");
            else
                ImGui::OpenPopup("Import Texture");
        }
        else
            ImGui::OpenPopup("Import Texture To All Meshes");
    }

    void TextureModal::Update(ResourceManager* p_resource, ::Mesh* p_mesh, Asset* asset_win) noexcept
    {
        if (m_open)
        {
            m_open = false;
            if (!m_texturePaths.empty())
                OpenTextureModal(p_mesh);
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        // Open texture import modal window
        // Select the type to import the texture
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Import Texture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const std::filesystem::path path{ m_texturePaths.front() };
            ImGui::TextUnformatted(path.filename().string().c_str());
            m_texTypeDropDown.Combo();
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                const auto tex = p_resource->LoadTexture(path.string().c_str());
                auto* texture = p_resource->GetTexture(tex);
                asset_win->AddTextureData(texture);
                switch (m_texTypeDropDown.GetSelectedIndex())
                {
                case 1: p_mesh->material.t_metallic = texture; break;
                case 2: p_mesh->material.t_roughness = texture; break;
                case 3: p_mesh->material.t_ao = texture; break;
                case 4: p_mesh->material.t_normal = texture; break;
                default:p_mesh->material.t_albedo = texture; break;
                }
                m_texturePaths.pop(); m_open = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                m_texturePaths.pop(); m_open = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // If the mesh is not set or it is the root mesh,
        //      apply the texture to all meshes
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Import Texture To All Meshes", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const std::filesystem::path path{ m_texturePaths.front() };
            ImGui::TextUnformatted(path.filename().string().c_str());
            m_texTypeDropDown.Combo();
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                const auto tex = p_resource->LoadTexture(path.string().c_str());
                auto* texture = p_resource->GetTexture(tex);
                asset_win->AddTextureData(texture);
                auto& meshes = m_p_object->m_p_model->m_meshes;
                switch (m_texTypeDropDown.GetSelectedIndex())
                {
                case 1:
                {
                    for (std::size_t i = 1; i < meshes.size(); ++i)
                        meshes[i].material.t_metallic = texture;
                }
                break;
                case 2:
                {
                    for (std::size_t i = 1; i < meshes.size(); ++i)
                        meshes[i].material.t_roughness = texture;
                }
                break;
                case 3:
                {
                    for (std::size_t i = 1; i < meshes.size(); ++i)
                        meshes[i].material.t_ao = texture;
                }
                break;
                case 4:
                {
                    for (std::size_t i = 1; i < meshes.size(); ++i)
                        meshes[i].material.t_normal = texture;
                }
                break;
                default:
                {
                    for (std::size_t i = 1; i < meshes.size(); ++i)
                        meshes[i].material.t_albedo = texture;
                }
                break;
                }
                m_texturePaths.pop(); m_open = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                m_texturePaths.pop(); m_open = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    /* Texture Modal Window - end -------------------------------------------------------------------*/

}
