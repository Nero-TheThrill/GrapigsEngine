/*
 *	Author		: Jinwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.cpp
 *	Desc		: Manage shader, object
 */
#include "ResourceManager.h"

#include "Input.h"

#include <iostream>
#include <stb_image.h>	// load png
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
/* Object - start -------------------------------------------------------------------------------*/

void Object::Draw(Primitive primitive, unsigned IBL) const noexcept
{
    m_p_shader->Use();
    
    m_p_shader->SendUniform("t_ibl", 55);
    m_p_shader->SendUniform("t_brdflut", 56);
    m_p_shader->SendUniform("u_color", m_color);
    m_p_shader->SendUniform("u_modelToWorld", m_transform.GetTransformMatrix());
    m_p_model->Draw(primitive, m_p_shader);
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
    glGenTextures(1, &m_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap);
    int width, height, nrChannels;
    std::vector<std::string> cubemap_paths
    {
        "texture/skybox/right.jpg",
        "texture/skybox/left.jpg",
        "texture/skybox/top.jpg",
        "texture/skybox/bottom.jpg",
        "texture/skybox/front.jpg",
        "texture/skybox/back.jpg"
    };

    for (unsigned int i = 0; i < cubemap_paths.size(); i++)
    {
        unsigned char* data = stbi_load(cubemap_paths[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "failed to load: " << cubemap_paths[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTextureUnit(55, m_cubemap);


    stbi_set_flip_vertically_on_load(true);
    int channels;
    unsigned char* data = stbi_load("texture/brdf.png", &width, &height, &channels, 0);
    if (data == nullptr)
    {
        std::cout << "[Texture] Error: Unable to load texture/brdf.png" << std::endl;
        return;
    }

    const GLenum sized_internal_format = (channels == 4) ? GL_RGBA8 : GL_RGB8;
    const GLenum base_internal_format = (channels == 4) ? GL_RGBA : GL_RGB;

    if (!m_brdf)
        glCreateTextures(GL_TEXTURE_2D, 1, &m_brdf);
    glTextureStorage2D(m_brdf, 1, sized_internal_format, width, height);
    glTextureSubImage2D(m_brdf, 0, 0, 0, width, height, base_internal_format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    glTextureParameteri(m_brdf, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_brdf, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_brdf, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_brdf, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTextureUnit(56, m_brdf);

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
    for (const auto& m : m_textures)
    {
        if (m.second->m_path == file_path)
            return m.second->m_tag;
    }

    // Load texture
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
    delete m_object;
    m_object = new Object();
    if(m_models.contains(mesh))
	    m_object->m_p_model = m_models[mesh];
    if(m_shaders.contains(shader))
	    m_object->m_p_shader = m_shaders[shader];
    if (m_textures.contains(texture))
    {
        auto& meshes = m_object->m_p_model->m_meshes;
        for(std::size_t i = 1; i < meshes.size(); ++i)
            meshes[i].material.t_albedo = m_textures[texture];
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
    m_object->Draw(Primitive::LineLoop, m_cubemap);
}

void ResourceManager::DrawTriangles() const noexcept
{
    m_fbo->Bind();
    m_object->Draw(Primitive::Triangles, m_cubemap);
    m_fbo->UnBind();
}

/* ResourceManager - end ------------------------------------------------------------------------*/