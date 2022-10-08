/*
 *	Author		: Jiwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.h
 *	Desc		: Manage shader, object
 */
#pragma once
#include "stb_image.h"
#include <vector>
#include <string>
#include <gl/glew.h>

#include "Transform.h"
#include "FBXImporter.h"

enum class LightType
{
    POINT,
    DIRECTIONAL,
    SPOT
};

struct Light
{
    Transform m_transform;
    LightType m_type = LightType::POINT;
    glm::vec3 m_direction = glm::vec3{ -1,1,1 }; // only for directional
    glm::vec3 m_ambient = glm::vec3{ 0.7f,0.7f,0.85f }, m_diffuse = glm::vec3{ 1,1,1 }, m_specular = glm::vec3{ 1,1,1 };// for all lights
    float m_inner_angle = 0.33f, m_outer_angle = 0.65f, m_falloff = 0.35f;
    glm::vec3 m_attenuation = glm::vec3(0.001f, 0.0005f, 0.00025f);
};
class Lights
{
public:
    void Init();
    void Update();
    void AddLight(Light light);
private:
    std::vector<Light> lights;
    GLuint m_ubo = 0;
};

class Object
{
public:
    Transform m_transform;
    unsigned m_tag = 0;
    std::string m_name;
    ShaderProgram* m_p_shader = nullptr;
    std::vector<MeshGroup*> m_p_meshGroups;
    glm::vec4 m_color = glm::vec4{1};

    void Draw(Primitive primitive) const noexcept;
    void SetTexture(unsigned texture_tag);
};


class ResourceManager
{
public:
    ResourceManager() = default;
    ~ResourceManager();

    void Clear() noexcept;

    unsigned LoadFbx(const char* fbx_file_path) noexcept;
    Object* LoadFbxAndCreateObject(const char* fbx_file_path, int shader_tag) noexcept;

    unsigned LoadTexture(const char* texture_file_path) noexcept;

    [[nodiscard]] std::vector<MeshGroup*> GetMeshByTag(const unsigned& target_tag) const noexcept;
    [[nodiscard]] Object* GetObjectByName(std::string target_name) const noexcept;
    [[nodiscard]] std::vector<Object*> GetObjectByTag(const unsigned& target_tag) const noexcept;
    [[nodiscard]] ShaderProgram* GetShaderByTag(unsigned tag) const noexcept;

    ShaderProgram* CreateShader(const std::vector<std::pair<ShaderType, std::filesystem::path>>& shader_files) noexcept;
    Object* CreateObject(unsigned tag, const std::string& name, unsigned mesh_tag, unsigned shader_tag) noexcept;
    void DeleteObject(Object* obj);

    void DrawLines() const noexcept;
    void DrawTriangles() const noexcept;

private:
    std::vector<Object*> obj_storage;
    std::vector<std::pair<std::string, unsigned>> texture_storage;
    std::vector<std::vector<MeshGroup*>> gmesh_storage;
    std::vector<ShaderProgram*> shader_storage;
    std::string log_string;
public:
    void SetGUIObject(Object* obj) noexcept;
    void UpdateObjectGUI() noexcept;
private:
    struct GUIObject
    {
        Object* object;
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scaling;
        float scale;
        float prevScale;
        void DrawGUI() noexcept;
    };
    GUIObject m_guiObject;
};

