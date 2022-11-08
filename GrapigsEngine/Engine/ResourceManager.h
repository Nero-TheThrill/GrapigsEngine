/*
 *	Author		: Jinwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.h
 *	Desc		: Manage shader, object
 */
#pragma once
#include <vector>
#include <string>
#include <gl/glew.h>

#include "Transform.h"
#include "FBXImporter.h"

#define ERROR_INDEX 9999

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
    glm::vec3 m_direction = glm::vec3{ 1,-0.1,-0.5 }; // only for directional
    glm::vec3 m_ambient = glm::vec3{ 0.9f,0.9f,0.95f }, m_diffuse = glm::vec3{ 1,1,1 }, m_specular = glm::vec3{ 1,1,1 };// for all lights
    float m_inner_angle = 0.33f, m_outer_angle = 0.65f, m_falloff = 0.35f;
    glm::vec3 m_attenuation = glm::vec3(0.0001f, 0.00005f, 0.000025f);
};

class Lights
{
public:
    Lights() noexcept;
    ~Lights() noexcept;
    void Update();
    void AddLight(Light light);
private:
    std::vector<Light> lights;
    GLuint m_ubo = 0;
};

class Grid
{
public:
    Grid(float size, std::size_t divide = 10) noexcept;
    ~Grid() noexcept;
    void Draw() const noexcept;
    glm::vec4 m_color{ 0, 0, 0, 0.5f };
private:
    const float m_height = 0.f;
    unsigned m_vao, m_vbo;
    ShaderProgram* m_program;
    std::vector<glm::vec3> m_position;
};

class Object
{
public:
    Transform m_transform;
    unsigned m_tag = 0;
    std::string m_name;
    ShaderProgram* m_p_shader = nullptr;
    Model* m_p_model = nullptr;
    glm::vec4 m_color = glm::vec4{1};

    void Draw(Primitive primitive, const std::map<TextureType, unsigned>& textures) const noexcept;
};

class ResourceManager
{
    friend class GUI;
    typedef std::pair<std::string, unsigned> TexturePathID;
public:
    ResourceManager();
    ~ResourceManager();

    void Clear() noexcept;

    unsigned LoadFbx(const char* path) noexcept;
    unsigned LoadTexture(const char* path) noexcept;
    unsigned LoadShaders(const std::vector<std::pair<ShaderType, std::filesystem::path>>& paths) noexcept;

    void AddTexture(Texture* texture) noexcept;
    Texture* GetTexture(const std::filesystem::path& path) const noexcept;

    Texture* GetTexture(const unsigned tag) noexcept;
    Object* CreateObject(unsigned mesh, unsigned shader, unsigned texture = ERROR_INDEX) noexcept;
    Object* CreateObject(const char* path) noexcept;
    void DrawLines() const noexcept;
    void DrawTriangles() const noexcept;

    static FrameBufferObject* m_fbo;
private:
    Grid* m_grid;
    Object* m_object;
    std::map<unsigned, Texture*> m_textures;
    std::map<unsigned, Model*> m_models;
    std::map<unsigned, ShaderProgram*> m_shaders;
    CubeMapTexture m_cubemap;
    Texture m_brdf;
    std::map<TextureType, unsigned> m_texUnit;
};