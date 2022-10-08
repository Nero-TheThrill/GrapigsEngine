/*
 *	Author		: Jiwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.cpp
 *	Desc		: Manage shader, object
 */
#include "ResourceManager.h"

#include <imgui.h>  // ImGui::
#include <iostream> // std::cout
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
        glBufferSubData(GL_UNIFORM_BUFFER, 16 + iter * lightstride + 32, sizeof(glm::vec3), &light.m_transform.GetPosition()[0]);
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
    m_p_shader->Use();
    m_p_shader->SendUniform("u_color", m_color);
    m_p_shader->SendUniform("u_modelToWorld", m_transform.GetTransformMatrix());
    for(const auto& m: m_p_meshGroups)
        m->Draw(primitive, m_p_shader);
    m_p_shader->UnUse();
}

void Object::SetTexture(unsigned texture_tag)
{
    for(auto gmesh : m_p_meshGroups)
    {
        for(Mesh& mesh: gmesh->m_meshes)
        {
            mesh.material.texture = texture_tag;
        }
    }
}

ResourceManager::~ResourceManager()
{
    Clear();
}

void ResourceManager::Clear() noexcept
{
    for (auto& group : gmesh_storage)
    {
        for (const auto& m : group)
            delete m;
        group.clear();
    }
    gmesh_storage.clear();
    for (const auto& s : shader_storage)
        delete s;
    shader_storage.clear();
    for (const auto& o : obj_storage)
        delete o;
    obj_storage.clear();
}

unsigned ResourceManager::LoadFbx(const char* fbx_file_path) noexcept
{
    const auto& meshes = FBXImporter::Load(fbx_file_path);
    const auto tag = static_cast<unsigned>(gmesh_storage.size());
    gmesh_storage.push_back(meshes);
    return tag;
}

Object* ResourceManager::LoadFbxAndCreateObject(const char* fbx_file_path, int shader_tag) noexcept
{
    const std::filesystem::path path{ fbx_file_path };
    return CreateObject(0, path.stem().string(), LoadFbx(fbx_file_path), shader_tag);
}

unsigned ResourceManager::LoadTexture(const char* texture_file_path) noexcept
{
    for(auto texture: texture_storage)
    {
        if (texture.first == texture_file_path)
        {
            return texture.second;
        }
    }
    int width, height, nrChannels;
    unsigned char* data;
    unsigned texture;
    stbi_set_flip_vertically_on_load(true);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps


    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.

    data = stbi_load(texture_file_path, &width, &height, &nrChannels, 0);


    if (data)
    {
        if (nrChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else if (nrChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << texture_file_path << std::endl;
        return 9999;
    }
    stbi_image_free(data);
    texture_storage.push_back(std::pair<std::string, unsigned>(texture_file_path, texture));
    return texture;
}

std::vector<MeshGroup*> ResourceManager::GetMeshByTag(const unsigned& target_tag) const noexcept
{
    if (target_tag < gmesh_storage.size())
        return gmesh_storage[target_tag];
    std::cout << "Mesh Not Found. Tag : " << target_tag << std::endl;
    return std::vector<MeshGroup*>{};
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

    new_obj->m_p_meshGroups = GetMeshByTag(mesh_tag);
    new_obj->m_p_shader = GetShaderByTag(shader_tag);
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

void ResourceManager::SetGUIObject(Object* obj) noexcept
{
    m_guiObject.object = obj;
    m_guiObject.position = obj->m_transform.GetPosition();
    m_guiObject.rotation = obj->m_transform.GetRotation();
    m_guiObject.scaling = obj->m_transform.GetScaling();
    m_guiObject.scale = 1;
    m_guiObject.prevScale = 1;
}

void ResourceManager::UpdateObjectGUI() noexcept
{
    ImGui::Begin("OBJECT");
    {
        ImGui::SetWindowSize(ImVec2(400, 1000));
        ImGui::SetWindowPos(ImVec2(0, 0));
        m_guiObject.DrawGUI();
    }
    ImGui::End();
}

void ResourceManager::GUIObject::DrawGUI() noexcept
{
    ImGui::TextColored(ImVec4{ 1, 1, 0, 1 }, object->m_name.c_str());
    if (ImGui::DragFloat3("Translation", &position[0], 0.01f, 0, 0, "%.2f"))
        object->m_transform.Translate(position);
    if (ImGui::DragFloat3("Rotation", &rotation[0], 1, 0, 0, "%.1f"))
    {
        if (rotation.x > 360)
            rotation.x = 0;
        else if (rotation.x < 0)
            rotation.x = 360;
        if (rotation.y > 360)
            rotation.y = 0;
        else if (rotation.y < 0)
            rotation.y = 360;
        if (rotation.z > 360)
            rotation.z = 0;
        else if (rotation.z < 0)
            rotation.z = 360;
        object->m_transform.Rotate(rotation.x, rotation.y, rotation.z);
    }
    if (ImGui::DragFloat3("Scaling", &scaling[0], 0.01f, 0, 0, "%.2f"))
    {
        if (scaling.x < std::numeric_limits<float>::epsilon())
            scaling.x = 0.001f;
        if (scaling.y < std::numeric_limits<float>::epsilon())
            scaling.y = 0.001f;
        if (scaling.z < std::numeric_limits<float>::epsilon())
            scaling.z = 0.001f;
        object->m_transform.Scale(scaling);
        scale = 1;
        prevScale = 1;
    }
    if(ImGui::DragFloat("Scale by", &scale, 0.01f, 0, 0, "%.2f"))
    {
        if (scale < std::numeric_limits<float>::epsilon())
            scale = 0.001f;
        
        scaling *= (scale / prevScale);
        object->m_transform.Scale(scaling * scale);
        prevScale = scale;
    }
}
