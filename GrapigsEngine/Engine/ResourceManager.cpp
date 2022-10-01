/*
 *	Author		: Jiwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.cpp
 *	Desc		: Manage shader, object
 */
#include "ResourceManager.h"
#include <iostream>


void Lights::Init()
{
    GLsizeiptr size = 128 * 16 + 16;// 16 * (sizeof(unsigned) + 5 * sizeof(glm::vec3) + 3 * sizeof(float)) + sizeof(unsigned)+3 * sizeof(float) + 3 * sizeof(glm::vec3);
    glGenBuffers(1, &m_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_ubo, 0, size);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Lights::Update()
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    auto lightnumber = static_cast<unsigned>(lights.size());
    GLsizeiptr lightstride = 128;// (sizeof(unsigned) + 5 * sizeof(glm::vec3) + 3 * sizeof(float));
    int iter = 0;

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(unsigned), &lightnumber);

    for (auto light : lights)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride, sizeof(unsigned), &(light.m_type));
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 16, sizeof(glm::vec3), &light.m_direction[0]);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 32, sizeof(glm::vec3), &light.m_transform.position[0]);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 48, sizeof(glm::vec3), &light.m_ambient);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 64, sizeof(glm::vec3), &light.m_diffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 80, sizeof(glm::vec3), &light.m_specular);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 96, sizeof(glm::vec3), &light.m_attenuation[0]);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 108, sizeof(float), &light.m_inner_angle);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 112, sizeof(float), &light.m_outer_angle);
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 116, sizeof(float), &light.m_falloff);
        iter++;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Lights::AddLight(Light light)
{
    lights.push_back(light);
}

void Object::Draw(Primitive primitive) const noexcept
{
    m_shader->Use();
    m_shader->SendUniform("u_color", m_color);
    m_shader->SendUniform("u_modelToWorld", m_transform.GetTransformMatrix());
    m_meshGroup->Draw(primitive, m_shader);
    m_shader->UnUse();
}

ResourceManager::~ResourceManager()
{
    Clear();
}

void ResourceManager::Clear() noexcept
{
    for (const auto& m : gmesh_storage)
        delete m;
    gmesh_storage.clear();
    for (const auto& s : shader_storage)
        delete s;
    shader_storage.clear();
    for (const auto& o : obj_storage)
        delete o;
    obj_storage.clear();
}

void ResourceManager::LoadFbx(const char* fbx_file_path) noexcept
{
    const auto& meshes = FBXImporter::Load(fbx_file_path);
    for(const auto& m : meshes)
    {
        const_cast<unsigned&>(m->m_tag) = static_cast<unsigned>(gmesh_storage.size());
        gmesh_storage.push_back(m);
    }
}

void ResourceManager::LoadFbxAndCreateObject(const char* fbx_file_path, int shader_tag) noexcept
{
    const auto& meshes = FBXImporter::Load(fbx_file_path);
    for (const auto& m : meshes)
    {
        const_cast<unsigned&>(m->m_tag) = static_cast<unsigned>(gmesh_storage.size());
        gmesh_storage.push_back(m);
        CreateObject(0, m->m_name, m->m_tag, shader_tag);
    }
}

MeshGroup* ResourceManager::GetMeshByName(const std::string& target_name) const noexcept
{
    for(const auto& mesh : gmesh_storage)
    {
        if(mesh->m_name== target_name)
            return mesh;
    }
    std::cout << "Mesh Not Found. Name : " << target_name << std::endl;
    return nullptr;
}
 
MeshGroup* ResourceManager::GetMeshByTag(const unsigned& target_tag) const noexcept
{
    for (const auto& mesh : gmesh_storage)
    {
        if (mesh->m_tag == target_tag)
            return mesh;
    }
    std::cout << "Mesh Not Found. Tag : " << target_tag << std::endl;
    return nullptr;
}

Object* ResourceManager::GetObjectByName(std::string target_name) const noexcept
{
    for (const auto& obj : obj_storage)
    {
        if (obj->m_name == target_name)
            return obj;
    }
    std::cout << "Object Not Found. Name : " << target_name << std::endl;
    return nullptr;
}

std::vector<Object*> ResourceManager::GetObjectByTag(const unsigned& target_tag) const noexcept
{
    std::vector<Object*> result;
    for (const auto& obj : obj_storage)
    {
        if (obj->m_tag == target_tag)
            result.push_back(obj);
    }
    if(result.empty())
        std::cout << "Object Not Found. Tag : " << target_tag << std::endl;
    return result;
}

ShaderProgram* ResourceManager::GetShaderByTag(unsigned tag) const noexcept
{
    return tag < shader_storage.size() ? shader_storage[tag] : nullptr;
}

ShaderProgram* ResourceManager::CreateShader(const std::vector<std::pair<ShaderType, std::filesystem::path>>& shader_files) noexcept
{
    ShaderProgram* program = new ShaderProgram(shader_files);
    const_cast<unsigned&>(program->m_tag) = static_cast<unsigned>(shader_storage.size());
    shader_storage.push_back(program);
    return program;
}

Object* ResourceManager::CreateObject(unsigned tag, const std::string& name, unsigned mesh_tag, unsigned shader_tag) noexcept
{
    Object* new_obj = new Object();

    new_obj->m_meshGroup = GetMeshByTag(mesh_tag);
    new_obj->m_shader = GetShaderByTag(shader_tag);
    const_cast<unsigned&>(new_obj->m_tag) = tag;
    new_obj->m_name = name;

    obj_storage.push_back(new_obj);
    return new_obj;
}

void ResourceManager::DeleteObject(Object* obj)
{
    if (auto target_obj = std::find(obj_storage.begin(), obj_storage.end(), obj);
        target_obj != obj_storage.end())
    {
        std::cout << "Object <name=" << (*target_obj)->m_name << " tag = " << (*target_obj)->m_tag << " > has been delete" << std::endl;
        obj_storage.erase(target_obj);
    }
}

void ResourceManager::DrawLines() const noexcept
{
    for (const auto& obj : obj_storage)
    {
        obj->Draw(Primitive::LineLoop);
    }
}

void ResourceManager::DrawTriangles() const noexcept
{
    for (const auto& obj : obj_storage)
    {
        obj->Draw(Primitive::Triangles);
    }
}
