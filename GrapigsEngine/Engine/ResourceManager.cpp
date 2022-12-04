/*
 *	Author		: Jinwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.cpp
 *	Desc		: Manage shader, object
 */
#include "ResourceManager.h"

#include <iostream>
#include <ranges>   // std::views::

#include "Camera.h"
#include "Input.h"

 /* Light - start --------------------------------------------------------------------------------*/

Lights::Lights() noexcept
{
    GLsizeiptr size = 128 * 16 + 16;// 16 * (sizeof(unsigned) + 5 * sizeof(glm::vec3) + 3 * sizeof(float)) + sizeof(unsigned)+3 * sizeof(float) + 3 * sizeof(glm::vec3);
    glGenBuffers(1, &m_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_ubo, 0, size);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Lights::~Lights() noexcept
{
    glDeleteBuffers(1, &m_ubo);
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
/* Grid - start ---------------------------------------------------------------------------------*/

Grid::Grid(float size, std::size_t divide) noexcept
    : m_vao(0), m_vbo(0), m_position((divide + 1) * 4)
{
    assert(divide > 2);

    const std::vector<std::pair<ShaderType, std::filesystem::path>> files = {
               std::make_pair(ShaderType::Vertex, "shader/line.vert"),
               std::make_pair(ShaderType::Fragment, "shader/line.frag")
    };
    m_program = new ShaderProgram(files);

    const float dx = (2.f * size) / static_cast<float>(divide);
    float x = -size;
    for (std::size_t i = 0; i <= divide; ++i)
    {
        m_position[i * 4] = glm::vec3{ x, m_height, -size };
        m_position[i * 4 + 1] = glm::vec3{ x, m_height, size };
        m_position[i * 4 + 2] = glm::vec3{ -size, m_height, x };
        m_position[i * 4 + 3] = glm::vec3{ size, m_height, x };
        x += dx;
    }

    glCreateBuffers(1, &m_vbo);
    glNamedBufferStorage(m_vbo, static_cast<GLsizeiptr>(sizeof(glm::vec3) * m_position.size()), m_position.data(), GL_DYNAMIC_STORAGE_BIT);
    glCreateVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Vertex Position
    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(glm::vec3));
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(m_vao, 0, 0);

    glBindVertexArray(0);
}

Grid::~Grid() noexcept
{
    delete m_program;
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

void Grid::Draw() const noexcept
{
    m_program->Use();
    glBindVertexArray(m_vao);
    m_program->SendUniform("u_color", m_color);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_position.size()));
    glBindVertexArray(0);
    m_program->UnUse();
}

/* Grid - end -----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* Object - start -------------------------------------------------------------------------------*/

void Object::Draw(Primitive primitive, const std::map<TextureType, unsigned>& textures) const noexcept
{
    m_p_shader->Use();

    m_p_shader->SendUniform("t_ibl", textures.find(TextureType::IBL)->second);
    m_p_shader->SendUniform("t_irradiance", textures.find(TextureType::Irradiance)->second);
    m_p_shader->SendUniform("t_brdflut", textures.find(TextureType::BRDF)->second);
    m_p_shader->SendUniform("t_environment", textures.find(TextureType::Environment)->second);
    m_p_shader->SendUniform("t_prefiltermap", textures.find(TextureType::PrefilterMap)->second);
    m_p_shader->SendUniform("u_color", m_color);
    m_p_shader->SendUniform("u_modelToWorld", m_transform.GetTransformMatrix());

    m_p_model->Draw(primitive, m_p_shader);

    m_p_shader->UnUse();
}


/* Object - end ---------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* ResourceManager - start ----------------------------------------------------------------------*/

FrameBufferObject* ResourceManager::m_fbo = new FrameBufferObject();
FrameBufferObject_PreFilterMap* ResourceManager::m_fbo_prefiltermap = new FrameBufferObject_PreFilterMap();
ResourceManager::ResourceManager() :
    m_grid(new Grid(3, 10)),
    m_object(nullptr),
    m_skybox(nullptr),
    m_cube(nullptr),
    m_brdf("texture/brdf.png"),
    m_hdr("texture/skybox/BasketballCourt_3k.hdr", false, true),
    m_environment("texture/skybox/BasketballCourt_8k.jpg"),
    m_irradiance("texture/skybox/BasketballCourt_Env.hdr", false, true)
{
    const glm::ivec2& size = Input::s_m_windowSize;
    m_fbo->Init(size.x, size.y);
    m_fbo_prefiltermap->Init(size.x, size.y);


    m_texUnit[TextureType::IBL] = m_hdr.Unit();
    m_texUnit[TextureType::Irradiance] = m_irradiance.Unit();
    m_texUnit[TextureType::BRDF] = m_brdf.Unit();
    m_texUnit[TextureType::Environment] = m_environment.Unit();

    glDepthFunc(GL_LEQUAL);
    CreateSkyBox();

    m_cube = new Object();
    m_cube->m_p_model = m_models[LoadFbx("model/skycube.fbx")];

    const std::vector<std::pair<ShaderType, std::filesystem::path>> shader_files = {
        std::make_pair(ShaderType::Vertex, "shader/prefilter.vert"),
        std::make_pair(ShaderType::Fragment, "shader/prefilter.frag")
    };
    m_cube->m_p_shader = m_shaders[LoadShaders(shader_files)];

    m_cube->m_p_shader->Use();
    m_fbo_prefiltermap->Bind();
    m_texUnit[TextureType::PrefilterMap] = m_fbo_prefiltermap->Unit();
    constexpr glm::vec3 views[] =
    {
        glm::vec3(-1,0,0),
        glm::vec3(1,0,0),
        glm::vec3(0,-1,-0.000001),
        glm::vec3(0,1,-0.000001),
        glm::vec3(0,0,-1),
        glm::vec3(0,0,1),
    };
    constexpr int maxMipLevels = 7;

    m_cube->m_p_shader->SendUniform("t_ibl", m_texUnit.find(TextureType::IBL)->second);
    m_cube->m_p_shader->SendUniform("t_irradiance", m_texUnit.find(TextureType::Irradiance)->second);
    m_cube->m_p_shader->SendUniform("t_brdflut", m_texUnit.find(TextureType::BRDF)->second);
    m_cube->m_p_shader->SendUniform("t_environment", m_texUnit.find(TextureType::Environment)->second);
    m_cube->m_p_shader->SendUniform("u_modelToWorld", m_cube->m_transform.GetTransformMatrix());

    for (int mip = 0; mip < maxMipLevels; mip++)
    {

        const unsigned int mipWidth = static_cast<unsigned int>(512 * std::pow(0.5, mip));
        const unsigned int mipHeight = static_cast<unsigned int>(512 * std::pow(0.5, mip));
        m_fbo_prefiltermap->BindRBO_PrefilterMap(mipWidth, mipHeight);


        float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        m_cube->m_p_shader->SendUniform("roughness", roughness);

        for (unsigned int i = 0; i < 6; i++)
        {
            CameraBuffer::s_m_aspectRatio = static_cast<float>(mipWidth) / static_cast<float>(mipHeight);
            CameraBuffer::s_m_camera->Set(views[i]);
            CameraBuffer::Bind();
            m_fbo_prefiltermap->BindTexture_PrefilterMap(mip, i);


            m_cube->m_p_model->Draw(Primitive::Triangles, m_cube->m_p_shader);
        }

    }

    m_fbo_prefiltermap->UnBind();
    m_cube->m_p_shader->UnUse();
    CameraBuffer::s_m_aspectRatio = static_cast<float>(size.x) / static_cast<float>(size.y);
    CameraBuffer::s_m_camera->Reset();
    CameraBuffer::Bind();
    glViewport(0,0,size.x, size.y);
}

ResourceManager::~ResourceManager()
{
    Clear();
}

void ResourceManager::Clear() noexcept
{
    for (auto& m : m_models)
        delete m.second;
    m_models.clear();
    for (auto& s : m_shaders)
        delete s.second;
    m_shaders.clear();
    for (auto& t : m_textures)
        delete t.second;
    m_textures.clear();
    delete m_object;
    m_object = nullptr;
    m_fbo->Clear();
    delete m_fbo;
    m_fbo = nullptr;
}

unsigned ResourceManager::LoadFbx(const char* path) noexcept
{
    // If it is already exist, load existing model
    const std::filesystem::path file_path{ path };
    for(const auto& m : m_models)
    {
        if (m.second->m_path == file_path)
            return m.second->m_tag;
    }

    // Load model
    const auto& model = FBXImporter::Load(path);
    if(model != nullptr)
    {
	    const auto tag = static_cast<unsigned>(m_models.size());
        const_cast<unsigned&>(model->m_tag) = tag;
        m_models[tag] = model;
        model->m_name = file_path.filename().string();
		return tag;
    }
    return ERROR_INDEX;
}

unsigned ResourceManager::LoadTexture(const char* path) noexcept
{
    // If it is already exist, load existing texture
    const std::filesystem::path file_path{ path };
    Texture* texture = GetTexture(file_path);
    if (texture)
        return texture->m_tag;

    // Load texture
    texture = new Texture(path);
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

void ResourceManager::AddTexture(Texture* texture) noexcept
{
    unsigned tag = texture->m_tag;
    while(m_textures.contains(tag))
    {
        tag++;
    }
    const_cast<unsigned&>(texture->m_tag) = tag;
    m_textures[tag] = texture;
}

Texture* ResourceManager::GetTexture(const std::filesystem::path& path) const noexcept
{
    for (const auto& m : std::views::values(m_textures))
    {
        if (m->m_path == path)
            return m;
    }
    return nullptr;
}

Texture* ResourceManager::GetTexture(const unsigned tag) noexcept
{
    if (tag != ERROR_INDEX && m_textures.contains(tag))
        return m_textures[tag];
    return nullptr;
}

Object* ResourceManager::CreateObject(unsigned mesh, unsigned shader, unsigned t_albedo, unsigned t_metallic, unsigned t_roughness) noexcept
{
    delete m_object;
    m_object = new Object();
    if(m_models.contains(mesh))
	    m_object->m_p_model = m_models[mesh];
    if(m_shaders.contains(shader))
	    m_object->m_p_shader = m_shaders[shader];
    if (m_textures.contains(t_albedo))
    {
        auto& meshes = m_object->m_p_model->m_meshes;
        for (std::size_t i = 1; i < meshes.size(); ++i)
            meshes[i].material.t_albedo = m_textures[t_albedo];
    }
    if (m_textures.contains(t_metallic))
    {
        auto& meshes = m_object->m_p_model->m_meshes;
        for (std::size_t i = 1; i < meshes.size(); ++i)
            meshes[i].material.t_metallic = m_textures[t_metallic];
    }
    if (m_textures.contains(t_roughness))
    {
        auto& meshes = m_object->m_p_model->m_meshes;
        for (std::size_t i = 1; i < meshes.size(); ++i)
            meshes[i].material.t_roughness = m_textures[t_roughness];
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

void ResourceManager::CreateSkyBox() noexcept
{
    m_skybox = new Object();
    m_skybox->m_p_model = m_models[LoadFbx("model/skycube.fbx")];

    const std::vector<std::pair<ShaderType, std::filesystem::path>> shader_files = {
            std::make_pair(ShaderType::Vertex, "shader/skybox.vert"),
            std::make_pair(ShaderType::Fragment, "shader/skybox.frag")
    };
    m_skybox->m_p_shader = m_shaders[LoadShaders(shader_files)];
  
}

void ResourceManager::DrawSkyBox() const noexcept
{
    m_skybox->Draw(Primitive::Triangles, m_texUnit);
}


void ResourceManager::DrawLines() const noexcept
{
    m_grid->Draw();
    m_object->Draw(Primitive::LineLoop, m_texUnit);
}

void ResourceManager::DrawTriangles() const noexcept
{
    m_fbo->Bind();
    glDepthMask(GL_FALSE);
    DrawSkyBox();
    glDepthMask(GL_TRUE);

    m_grid->Draw();
    m_object->Draw(Primitive::Triangles, m_texUnit);
    m_fbo->UnBind();
}

/* ResourceManager - end ------------------------------------------------------------------------*/