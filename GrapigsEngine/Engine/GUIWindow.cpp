#include "GUIWindow.h"

#pragma warning (disable : 4201)

#include <imgui.h>              // ImGui functions
#include <glm/gtc/type_ptr.hpp>          // glm::value_ptr

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

    glm::vec3 ClampTo360(const glm::vec3 vec)
    {
        return glm::vec3{ ClampTo360(vec.x), ClampTo360(vec.y), ClampTo360(vec.z) };
    }

    bool is_equal(const glm::vec4& v1, const glm::vec4& v2)
    {
        constexpr float epsilon = std::numeric_limits<float>::epsilon();
        return glm::epsilonEqual(v1[0], v2[0], epsilon)
            && glm::epsilonEqual(v1[1], v2[1], epsilon)
            && glm::epsilonEqual(v1[2], v2[2], epsilon)
            && glm::epsilonEqual(v1[3], v2[3], epsilon);
    }

    bool is_equal(const glm::mat4 m1, const glm::mat4& m2)
    {
        return is_equal(m1[0], m2[0]) && is_equal(m1[1], m2[1]) && is_equal(m1[2], m2[2]) && is_equal(m1[3], m2[3]);
    }

	void HelpMarker(const char* desc)
    {
        //ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
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
        m_id.clear();
        m_dataSize = 0;
        m_selected = 0;
    }

    void DropDown::AddData(const char* datum, unsigned tag) noexcept
    {
        m_dataSize++;
        m_data.push_back(datum);
        m_id.push_back(tag);
    }

    bool DropDown::Combo() noexcept
    {
        return ImGui::Combo(m_label.c_str(), &m_selected, m_data.data(), m_dataSize);
    }

    bool DropDown::Selectable() noexcept
    {
        bool changed = false;
        for(int i = 0; i < m_dataSize; ++i)
        {
            if (ImGui::Selectable(m_data[i], m_selected == i))
            {
	            m_selected = i;
                changed = true;
            }
        }
        return changed;
    }

    int DropDown::GetSelectedIndex() const noexcept
    {
        return m_selected;
    }

    std::string DropDown::GetSelectedString() const noexcept
    {
        return m_data[m_selected];
    }

    unsigned DropDown::GetSelectedID() const noexcept
    {
        return m_id[m_selected];
    }

    /* DropDown - end -------------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------------------------------*/
	/* GUI::Window - start --------------------------------------------------------------------------*/

    Window::Window(const char* name, WindowInst* p_inst) noexcept
        : m_p_windows(p_inst), m_name(name)
    {
    }

    void Window::Update() noexcept
    {
        if (m_open)
        {
            ImGui::Begin(m_name.c_str(), &m_open);
            Content();
            ImGui::End();
        }
    }

    void Window::SetObject(Object* p_object) noexcept
    {
        m_p_object = p_object;
    }

    WindowInst::WindowInst(ResourceManager* p_resource) noexcept
        :
		m_transformWin("Transform", this),
        m_materialWin("Material", this),
        m_sceneWin("Scene", this),
        m_meshWin("Mesh", this),
		m_textureModal("Import Texture Modal", this),
        m_assetWin("Imported Asset", this),
        m_gizmoToolWin("Tool", this),
		m_p_resource(p_resource)
    {
    }

    void WindowInst::SetObject(Object* p_object) noexcept
    {
        m_transformWin.SetObject(p_object);
        m_materialWin.SetObject(p_object);
        m_sceneWin.SetObject(p_object);
        m_meshWin.SetObject(p_object);
        m_textureModal.SetObject(p_object);
        m_assetWin.SetObject(p_object);
    }

    void WindowInst::Update() noexcept
    {
        m_texturePreview.Update();
        m_transformWin.Update();
        m_materialWin.Update();
        m_meshWin.Update();
        m_textureModal.Update();
        m_assetWin.Update();
        m_sceneWin.Update();
    }

    /* GUI::Window - end ----------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------------------------------*/
	/* Transform Window - start ---------------------------------------------------------------------*/

	Transform::Transform(const char* name, WindowInst* p_inst) noexcept : Window(name, p_inst) {    }

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
        if(ImGui::Button("Reset"))
        {
            ::Transform& t = m_p_object->m_transform;
            t.Translate({ 0, 0, 0 });
            t.Rotate(0, 0, 0);
            t.Scale(1);
            SetObject(m_p_object);
            m_reset = true;
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

    bool Transform::DoesReset() noexcept
    {
        if(m_reset)
        {
            m_reset = false;
            return true;
        }
        return false;
    }

    /* Transform Window - end -----------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Gizmo Tool Window - start --------------------------------------------------------------------*/

    GizmoTool::GizmoTool(const char* name, WindowInst* p_inst) noexcept
        : Window(name, p_inst), m_selected(0),
		m_texture{ Texture("texture/Tool/cursor.png"),
				Texture("texture/Tool/translation.png"),
				Texture("texture/Tool/rotation.png"),
				Texture("texture/Tool/scaling.png") },
		m_texId{ static_cast<intptr_t>(m_texture[0].Handle()),
			static_cast<intptr_t>(m_texture[1].Handle()),
			static_cast<intptr_t>(m_texture[2].Handle()),
			static_cast<intptr_t>(m_texture[3].Handle()),}
    {
    }

    void GizmoTool::Update() noexcept
    {
        const auto pos = ImGui::GetWindowPos();
        ImGui::SetNextWindowPos(ImVec2{ pos.x + 10, pos.y + 30 });
        ImGui::BeginChild(m_name.c_str(), ImVec2{70, 250});
        Content();
        ImGui::EndChild();
    }

    void GizmoTool::Content() noexcept
    {
        constexpr ImVec2 uv0(0, 1);
        constexpr ImVec2 uv1(1, 0);
        constexpr ImVec2 size(50, 50);
        constexpr ImVec4 default_color(1, 1, 1, 1);
        constexpr ImVec4 selected_color(0, 1, 0, 1);
        for(int i = 0; i < 4; ++i)
        {
            ImGui::PushID(i);
            if (ImGui::ImageButton("", reinterpret_cast<void*>(m_texId[i]), size, uv0, uv1, ImVec4(0, 0, 0, 0), (i == m_selected) ? selected_color : default_color))
            {
                m_selected = i;
                switch (m_selected)
                {
                case 1: m_operation = Operation::Translation; break;
                case 2: m_operation = Operation::Rotation; break;
                case 3: m_operation = Operation::Scaling; break;
                default: m_operation = Operation::None; break;
                }
            }
            ImGui::PopID();
        }
    }

    /* Gizmo Tool Window - end ----------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Scene Window - start -------------------------------------------------------------------------*/

    Scene::Scene(const char* name, WindowInst* p_inst) noexcept : Window(name, p_inst)
    {
        auto* p_cam = CameraBuffer::GetMainCamera();
        m_view = p_cam->GetWorldToCameraMatrix();
        auto [near, far] = p_cam->Plane();
        m_proj = glm::perspective(glm::radians(p_cam->FOV()), CameraBuffer::s_m_aspectRatio, near, far);
    }

    void Scene::SetObject(Object* p_object) noexcept
    {
        Window::SetObject(p_object);
        m_model = m_p_object->m_transform.GetTransformMatrix();
        m_delta = glm::mat4{ 1 };
    }

    void Scene::Update() noexcept
    {
        if(m_open)
        {
            ImGuizmo::BeginFrame();
            ImGui::Begin(m_name.c_str(), &m_open);
	        {
		        Content();
            	m_p_windows->m_gizmoToolWin.Update();
            	UpdateGizmo();
	        }
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
        ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

        // Camera
        Camera* p_cam = CameraBuffer::GetMainCamera();
        if (p_cam->IsUpdated())
        {
	        m_view = CameraBuffer::GetMainCamera()->GetWorldToCameraMatrix();
            const auto [near, far] = p_cam->Plane();
            const auto size = ImGui::GetContentRegionAvail();
            m_proj = glm::perspective(glm::radians(p_cam->FOV()), size.x / size.y, near, far);
        }

        GUIWindow::GizmoTool* p_gizmotool = &m_p_windows->m_gizmoToolWin;
        GUIWindow::Transform* p_transformWin = &m_p_windows->m_transformWin;

        if(p_gizmotool->m_operation != GizmoTool::Operation::None)
        {
        	if(p_transformWin->DoesReset())
        	{
        		m_model = m_p_object->m_transform.GetTransformMatrix();
                m_delta = glm::mat4{ 1 };
        	}

        	// Set operation
        	auto operation = ImGuizmo::OPERATION::TRANSLATE;
        	if (p_gizmotool->m_operation == GizmoTool::Operation::Rotation)
        		operation = ImGuizmo::OPERATION::ROTATE;
        	else if (p_gizmotool->m_operation == GizmoTool::Operation::Scaling)
        		operation = ImGuizmo::OPERATION::SCALE;

        	ImGuizmo::Manipulate(glm::value_ptr(m_view), glm::value_ptr(m_proj), operation, ImGuizmo::MODE::WORLD, glm::value_ptr(m_model), glm::value_ptr(m_delta));
        	if(ImGuizmo::IsUsing())
        	{
        		glm::vec3 translation, rotation, scaling;
           
        		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(m_model), &translation[0], &rotation[0], &scaling[0]);
        		ImGuizmo::RecomposeMatrixFromComponents(&translation[0], &rotation[0], &scaling[0], glm::value_ptr(m_model));

        		::Transform& t = m_p_object->m_transform;
        		const glm::vec3 deltaRot = rotation - t.GetRotation();
        		rotation = (t.GetRotation() + deltaRot);

        		t.Translate(translation);
        		t.Rotate(rotation.x, rotation.y, rotation.z);
        		t.Scale(scaling);
                p_transformWin->SetObject(m_p_object);
        	}
        }

        const auto& size = ImGui::GetWindowSize();
        const auto& pos = ImGui::GetWindowPos();
        ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
        ImGuizmo::ViewManipulate(glm::value_ptr(m_view), glm::length(p_cam->Eye()), ImVec2(pos.x + size.x - 128, pos.y + 20), ImVec2(128, 128), 0x10101010);
    }

    /* Scene Window - end ---------------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Mesh Window - start --------------------------------------------------------------------------*/

    Mesh::Mesh(const char* name, WindowInst* p_inst) noexcept : Window(name, p_inst) {    }

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

    Material::Material(const char* name, WindowInst* p_inst) noexcept
		: Window(name, p_inst)
    {
    }

    void Material::Content() noexcept
    {
        // Detect drag and drop mesh from mesh window
        ImGui::Selectable(m_notice.c_str());
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_MeshNode"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                const int index = *(const int*)payload->Data;
                m_p_mesh = &m_p_object->m_p_model->m_meshes[index];
                m_notice = m_p_mesh->name;
            }
            ImGui::EndDragDropTarget();
        }

        // Show mesh information
        if (m_p_mesh)
        {
            ImGui::Separator();
            ::Material* m = &m_p_mesh->material;
            ImGui::SliderFloat("Metallic", &m->metallic, 0, 1, "%.2f");
            ImGui::SliderFloat("Roughness", &m->roughness, 0, 1, "%.2f");
            DrawTexture(m->t_albedo, "Color"); ImGui::SameLine();
            DrawTexture(m->t_metallic, "Metalness"); ImGui::SameLine();
            DrawTexture(m->t_roughness, "Roughness"); ImGui::SameLine();
            DrawTexture(m->t_normal, "Normal"); ImGui::SameLine();
            DrawTexture(m->t_ao, "AO");
        }
    }

    void Material::SetObject(Object* p_object) noexcept
    {
        Window::SetObject(p_object);
        m_notice = "<Drag and Drop Mesh from Mesh Window>";
    }

    ::Mesh* Material::GetMesh() const noexcept
    {
        return m_p_mesh;
    }

    void Material::DrawTexture(Texture* texture, const char* desc) noexcept
    {
        ImGui::BeginChild(desc, { 120, 150 }, true);
        ImGui::TextUnformatted(desc);
        if(texture)
        {
	        void* id = reinterpret_cast<void*>(static_cast<intptr_t>(texture->Handle()));
            if(ImGui::ImageButton(desc, id, { 100, 100 }, { 0, 1 }, { 1, 0 }, {0, 0, 0, 0}, {1, 1, 1, 1}))
            {
                ImGui::OpenPopup("Open Texture Menu");
                m_p_clicked = texture;
            }

            // Drag and drop detect
        	if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TextureSource"))
                {
                    IM_ASSERT(payload->DataSize == sizeof(unsigned));
                    auto* p_texture = m_p_windows->m_p_resource->GetTexture(*(unsigned*)(payload->Data));

                	::Material* m = &m_p_mesh->material;
                    if (m->t_albedo == texture) m->t_albedo = p_texture;
                    else if (m->t_normal == texture) m->t_normal = p_texture;
                    else if (m->t_metallic == texture) m->t_metallic = p_texture;
                    else if (m->t_roughness == texture) m->t_roughness = p_texture;
                    else if (m->t_ao == texture) m->t_ao = p_texture;
                }
                ImGui::EndDragDropTarget();
            }
        }

        if(ImGui::BeginPopup("Open Texture Menu"))
        {
            if(ImGui::Button("Preview"))
            {
                m_p_windows->m_texturePreview.AddTexture(m_p_clicked);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::EndChild();
    }

    /* Material Window - end ------------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Asset Window - start -------------------------------------------------------------------------*/

    Asset::Asset(const char* name, WindowInst* p_inst) noexcept
        : Window(name, p_inst), m_modelDD("Models"), m_textureDD("Textures"), m_p_texture(nullptr)
    {
    }

    void Asset::Content() noexcept
    {
        m_modelDD.Combo();
        ImGui::TextUnformatted("[THIS DROP DOWN DOES NOT WORK FOR NOW]");

        ImGui::Separator();
    	ImGui::Separator();

    	if(m_textureDD.Combo())
    	{
    		m_p_texture = m_p_windows->m_p_resource->GetTexture(m_textureDD.GetSelectedID());
    	}

        if(m_p_texture)
        {
            constexpr ImVec2 uv0(0, 1), uv1(1, 0), size(100, 100);
            ImGui::PushID("Asset-Texture");
            ImGui::ImageButton("", reinterpret_cast<void*>(static_cast<intptr_t>(m_p_texture->Handle())), size, uv0, uv1);
            // Dragging
        	if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("_TextureSource", &m_p_texture->m_tag, sizeof(unsigned));
                ImGui::Text(m_p_texture->m_name.c_str());
                ImGui::EndDragDropSource();
            }
            ImGui::PopID();

            // Set texture type
            ImGui::SameLine();
            ImGui::BeginChild("Texture Type", ImVec2{0, 100}, true, ImGuiWindowFlags_AlwaysAutoResize);
            m_p_windows->m_textureModal.m_texTypeDropDown.Selectable();
            ImGui::EndChild();

            // Apply texture
            if(ImGui::Button("Apply to all meshes"))
            {
                auto* texture = m_p_windows->m_p_resource->GetTexture(m_textureDD.GetSelectedID());
	            m_p_windows->m_textureModal.ApplyTextureToObject(texture);
            }
            if(ImGui::Button("Apply to current mesh"))
            {
                auto* texture = m_p_windows->m_p_resource->GetTexture(m_textureDD.GetSelectedID());
                if(const auto* mesh = m_p_windows->m_materialWin.GetMesh(); mesh)
                {
                    if(mesh->parent == -1)
	                    m_p_windows->m_textureModal.ApplyTextureToObject(texture);
                    else
		                m_p_windows->m_textureModal.ApplyTextureToMesh(texture);
                }
                else
                    m_p_windows->m_textureModal.ApplyTextureToObject(texture);
            }
        }
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
        m_modelDD.AddData(p_model->m_name.c_str(), p_model->m_tag);
    }

    void Asset::AddTextureData(Texture* p_texture) noexcept
    {
        for (const auto& tag : m_textureTags)
        {
            if (p_texture->m_tag == tag)
                return;
        }
        m_textureTags.insert(p_texture->m_tag);
        m_textureDD.AddData(p_texture->m_name.c_str(), p_texture->m_tag);

        if (m_textureTags.size() == 1)
            m_p_texture = p_texture;
    }

    /* Asset Window - end ---------------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Texture Modal Window - start -----------------------------------------------------------------*/

	TextureModal::TextureModal(const char* name, WindowInst* p_inst) noexcept
        :  Window(name, p_inst), m_isNewTexture(false), m_applyTexture(true),
			m_p_texture(nullptr), m_texTypeDropDown("Type")
    {
        m_open = false;
        m_texTypeDropDown.AddData("Albedo(default)");
        m_texTypeDropDown.AddData("Metalness");
        m_texTypeDropDown.AddData("Roughness");
        m_texTypeDropDown.AddData("Ambient Occlusion");
        m_texTypeDropDown.AddData("Normal Map");
    }

	void TextureModal::ImportTexture(const std::filesystem::path& path) noexcept
    {
        m_texturePaths.push(path);
        m_open = true;
    }

    void TextureModal::OpenTextureModal() noexcept
    {
        const ::Mesh* p_mesh = m_p_windows->m_materialWin.GetMesh();
        if (p_mesh != nullptr)
        {
            if (p_mesh->parent == -1) // root
                ImGui::OpenPopup("Import Texture To All Meshes");
            else
                ImGui::OpenPopup("Import Texture");
        }
        else
            ImGui::OpenPopup("Import Texture To All Meshes");

        // Check if this texture is already exist
        m_p_texture = m_p_windows->m_p_resource->GetTexture(m_texturePaths.front());
        if (m_p_texture == nullptr)
        {
            m_p_texture = new Texture(m_texturePaths.front().string().c_str());
            m_isNewTexture = true;
        }
        else
            m_isNewTexture = false;
    }
    
    void TextureModal::Update() noexcept
    {
        Content();
    }

    void TextureModal::Content() noexcept
    {
        if (m_open)
        {
            m_open = false;
            if (!m_texturePaths.empty())
                OpenTextureModal();
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        // Open texture import modal window
        // Select the type to import the texture
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Import Texture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            auto func = [&](::Texture* texture) {this->ApplyTextureToMesh(texture); };
            ModalContents(func);
            ImGui::EndPopup();
        }

        // If the mesh is not set or it is the root mesh,
        //      apply the texture to all meshes
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Import Texture To All Meshes", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            auto func = [&](::Texture* texture) {this->ApplyTextureToObject(texture); };
            ModalContents(func);
            ImGui::EndPopup();
        }
    }

    void TextureModal::ModalContents(std::function<void(Texture*)> apply_func) noexcept
    {
        // Show dropped texture information
        ImGui::TextUnformatted(m_p_texture->m_name.c_str());
        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(m_p_texture->Handle())), { 200, 200 }, { 0, 1 }, { 1, 0 });
        m_texTypeDropDown.Combo();
        ImGui::Separator();

        ImGui::Checkbox("Apply", &m_applyTexture);
        HelpMarker("Apply this texture to selected mesh on material window.\nIf mesh is not selected or Root is selected, the texture applies to all meshes of the object.");

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {   // Import texture
            if (m_isNewTexture)
            {   
                m_p_windows->m_p_resource->AddTexture(m_p_texture);
                m_p_windows->m_assetWin.AddTextureData(m_p_texture);
            }
            if (m_applyTexture)
                apply_func(m_p_texture);
            m_texturePaths.pop(); m_open = true;
            m_p_texture = nullptr;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {   // Do not import texture
            m_texturePaths.pop(); m_open = true;
            delete m_p_texture;
            m_p_texture = nullptr;
            ImGui::CloseCurrentPopup();
        }
    }

    void TextureModal::ApplyTextureToMesh(Texture* p_texture) const noexcept
    {
        ::Mesh* p_mesh = m_p_windows->m_materialWin.GetMesh();
        switch (m_texTypeDropDown.GetSelectedIndex())
        {
        case 1: p_mesh->material.t_metallic = p_texture; break;
        case 2: p_mesh->material.t_roughness = p_texture; break;
        case 3: p_mesh->material.t_ao = p_texture; break;
        case 4: p_mesh->material.t_normal = p_texture; break;
        default:p_mesh->material.t_albedo = p_texture; break;
        }
    }

    void TextureModal::ApplyTextureToObject(Texture* p_texture) const noexcept
    {
        auto& meshes = m_p_object->m_p_model->m_meshes;
        switch (m_texTypeDropDown.GetSelectedIndex())
        {
        case 1: { for (std::size_t i = 1; i < meshes.size(); ++i) meshes[i].material.t_metallic = p_texture; } break;
        case 2: { for (std::size_t i = 1; i < meshes.size(); ++i) meshes[i].material.t_roughness = p_texture; } break;
        case 3: { for (std::size_t i = 1; i < meshes.size(); ++i) meshes[i].material.t_ao = p_texture; } break;
        case 4: { for (std::size_t i = 1; i < meshes.size(); ++i) meshes[i].material.t_normal = p_texture; } break;
        default:{ for (std::size_t i = 1; i < meshes.size(); ++i) meshes[i].material.t_albedo = p_texture; } break;
        }
    }

    /* Texture Modal Window - end -------------------------------------------------------------------*/
    /*-----------------------------------------------------------------------------------------------*/
    /* Texture Preview Window - start ---------------------------------------------------------------*/

    void TexturePreview::Update()
    {
        for(auto iter = m_textures.begin(); iter != m_textures.end();)
        {
            const ::Texture* p_texture = iter->first;
            ImGui::Begin(p_texture->m_name.c_str(), &iter->second);
            const auto& size = ImGui::GetContentRegionAvail();
            const float min = std::min(size.x, size.y);
            ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(p_texture->Handle())), { min, min }, { 0, 1 }, { 1, 0 });
            ImGui::End();

            iter = (iter->second) ? std::next(iter) : m_textures.erase(iter);
        }
    }

    void TexturePreview::AddTexture(Texture* p_texture) noexcept
    {
        m_textures[p_texture] = true;
        // Set initial window size and pos
        ImGui::SetNextWindowSize({ 400, 400 });
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));
        ImGui::Begin(p_texture->m_name.c_str(), &m_textures[p_texture]);
        ImGui::End();
    }

    /* Texture Preview Window - end -----------------------------------------------------------------*/
}
