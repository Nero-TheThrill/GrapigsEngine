/*
 *	Author		: Jinwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.cpp
 *	Desc		: Manage shader, object
 */
#include "ResourceManager.h"

#include "Input.h"

 /* Light - start --------------------------------------------------------------------------------*/

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


/* Light - end ----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* Object - start -------------------------------------------------------------------------------*/

void Object::Draw(Primitive primitive) const noexcept
{
    m_p_shader->Use();
    m_p_shader->SendUniform("u_color", m_color);
    m_p_shader->SendUniform("u_modelToWorld", m_transform.GetTransformMatrix());
    m_p_mesh->Draw(primitive, m_p_shader);
    m_p_shader->UnUse();
}

/* Object - end ---------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* ResourceManager - start ----------------------------------------------------------------------*/

FrameBufferObject* ResourceManager::m_fbo = new FrameBufferObject();

ResourceManager::ResourceManager() :
	m_object(nullptr)
{
    const glm::ivec2& size = Input::s_m_windowSize;
    m_fbo->Init(size.x, size.y);
}

ResourceManager::~ResourceManager()
{
    Clear();
}

void ResourceManager::Clear() noexcept
{
    for (auto& m : m_meshes)
        delete m.second;
    m_meshes.clear();
    for (auto& s : m_shaders)
        delete s.second;
    m_shaders.clear();
    for (auto& t : m_textures)
        delete t.second;
    m_textures.clear();
    delete m_object;
    m_object = nullptr;
    m_fbo->Clear();
}

unsigned ResourceManager::LoadFbx(const char* path) noexcept
{
    const auto& mesh = FBXImporter::Load(path);
    if(mesh != nullptr)
    {
	    const auto tag = static_cast<unsigned>(m_meshes.size());
        const_cast<unsigned&>(mesh->m_tag) = tag;
        m_meshes[tag] = mesh;
		return tag;
    }
    return ERROR_INDEX;
}

unsigned ResourceManager::LoadTexture(const char* path) noexcept
{
    auto* texture = new Texture(path);
    if(texture->m_initialized)
    {
        const auto tag = static_cast<unsigned>(m_textures.size());
        const_cast<unsigned&>(texture->m_tag) = tag;
        m_textures[tag] = texture;
        return tag;
    }
    delete texture;
    return ERROR_INDEX;
}

unsigned ResourceManager::LoadShaders(const std::vector<std::pair<ShaderType, std::filesystem::path>>& paths) noexcept
{
    auto* program = new ShaderProgram(paths);
    const auto& tag = static_cast<unsigned>(m_shaders.size());
    const_cast<unsigned&>(program->m_tag) = tag;
    m_shaders[tag] = program;
    return tag;
}

Texture* ResourceManager::GetTexture(const unsigned tag) noexcept
{
    if (tag != ERROR_INDEX && m_textures.contains(tag))
        return m_textures[tag];
    return nullptr;
}

Object* ResourceManager::CreateObject(unsigned mesh, unsigned shader, unsigned texture) noexcept
{
    if (m_object != nullptr)
        delete m_object;

    m_object = new Object();
    if(m_meshes.contains(mesh))
	    m_object->m_p_mesh = m_meshes[mesh];
    if(m_shaders.contains(shader))
	    m_object->m_p_shader = m_shaders[shader];
    if (m_textures.contains(texture))
    {
        for(auto& m: m_object->m_p_mesh->m_meshes)
            m.material.t_albedo = m_textures[texture];
    }
    return  m_object;
}

Object* ResourceManager::CreateObject(const char* path) noexcept
{
    auto tag = LoadFbx(path);
    if(tag != ERROR_INDEX)
    {
        auto shader = m_object->m_p_shader->m_tag;
        CreateObject(tag, shader, ERROR_INDEX);
    }
    return m_object;
}



void ResourceManager::DrawLines() const noexcept
{
    m_object->Draw(Primitive::LineLoop);
}

void ResourceManager::DrawTriangles() const noexcept
{
    m_fbo->Bind();
    m_object->Draw(Primitive::Triangles);
    m_fbo->UnBind();
}

/* ResourceManager - end ------------------------------------------------------------------------*/