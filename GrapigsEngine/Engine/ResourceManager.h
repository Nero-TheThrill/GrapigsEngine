/*
 *	Author		: Jiwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.h
 *	Desc		: Manage shader, object
 */
#pragma once
#include <vector>
#include <string>

#include "Transform.h"
#include "FBXImporter.h"

class Object
{
public:
    Transform m_transform;
    unsigned m_tag = 0;
    std::string m_name;
    ShaderProgram* m_shader = nullptr;
    MeshGroup* m_meshGroup = nullptr;
    glm::vec4 m_color = glm::vec4{1};

    void Draw(Primitive primitive) const noexcept;
};


class ResourceManager
{
public:
    ResourceManager() = default;
    ~ResourceManager();

    void Clear() noexcept;

    void LoadFbx(const char* fbx_file_path) noexcept;
    void LoadFbxAndCreateObject(const char* fbx_file_path, int shader_tag) noexcept;

    [[nodiscard]] MeshGroup* GetMeshByName(const std::string& target_name) const noexcept;
    [[nodiscard]] MeshGroup* GetMeshByTag(const unsigned& target_tag) const noexcept;
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
    std::vector<MeshGroup*> gmesh_storage;
    std::vector<ShaderProgram*> shader_storage;
    std::string log_string;

};

