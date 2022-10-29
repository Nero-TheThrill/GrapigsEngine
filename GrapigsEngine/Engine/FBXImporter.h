/*
 *	Author		: Jina Hyun
 *	Date		: 09/29/22
 *	File Name	: FBXImporter.h
 *	Desc		: Import fbx
 */
#pragma once
#include <fbxsdk.h>	// Fbx variables and functions
#include <vector>	// std::vector
#include <glm/glm.hpp>	// glm
#include "Shader.h" // ShaderProgram

#define ERROR_INDEX 9999

enum class Primitive
{
    Lines = 0x001, LineLoop = 0x002, LineStrip = 0x003, Triangles = 0x004, TriangleStrip = 0x005, TriangleFan = 0x006

};

struct Vertex
{
	glm::vec4 position{};
	glm::vec4 vertex_normal{};
	glm::vec4 face_normal{};
    glm::vec2 texture_coordinate{};
};

struct Material
{
    float metallic = 0.f;
    float roughness = 0.f;
    glm::vec3 albedo = glm::vec3(1);
    Texture* t_albedo = nullptr;
    Texture* t_metallic = nullptr;
    Texture* t_roughness = nullptr;
    Texture* t_ao = nullptr;
    Texture* t_normal = nullptr;
};
struct Mesh
{
    std::string name{};
    glm::mat4 transform{ 1 };
    std::vector<Vertex> vertices;
    std::vector<int> children;
    Material material;
    int parent = -1;
    int index = 0;
};

class MeshGroup
{
public:
    MeshGroup() = default;
    ~MeshGroup();
    void InitBuffers(int largest_index) noexcept;
    void Clear() noexcept;
    void Draw(Primitive primitive, ShaderProgram* program) noexcept;

    std::string m_name{};
    int m_root = -1;
    std::vector<Mesh> m_meshes;
    const unsigned m_tag = 0;
private:
    void Draw(Primitive primitive, ShaderProgram* program, int index, glm::mat4 transform) const noexcept;
    unsigned m_vao = 0, m_vbo = 0;
};

class FBXImporter
{
public:
	static MeshGroup* Load(const char* file_path) noexcept;
private:
    static FbxScene* ImportFbx(FbxManager* p_manager, const char* file_path) noexcept;
	static MeshGroup* Parse(FbxNode* p_root) noexcept;
	static int ParseNode(FbxNode* p_node, int parent, std::vector<Mesh>& meshes) noexcept;
	static std::vector<Vertex> GetVertices( FbxMesh* p_mesh) noexcept;
    static std::size_t s_m_verticesCount;
    static int s_m_meshIndex;
};
