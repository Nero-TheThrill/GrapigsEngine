#pragma once
/*
 *	Author		: Jina Hyun
 *	Date		: 09/29/22
 *	File Name	: FBXImporter.h
 *	Desc		: Import fbx
 */
#include <fbxsdk.h>	// Fbx variables and functions
#include <vector>	// std::vector
#include <glm/glm.hpp>	// glm
#include <gl/glew.h>

#define NUM_VBO 3

struct Vertex
{
	glm::vec4 position{};
	glm::vec4 vertex_normal{};
	glm::vec4 face_normal{};
};

struct Mesh
{
	std::string name{};
	glm::mat4 transform{ 1 };
	std::vector<Vertex> verticies;
	std::vector<int> childrenIndex;
	int parentIndex = -1;

    GLuint VBO;

    void PopulateBuffers() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticies[0]) * verticies.size(), verticies.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 48, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 48, (void*)16);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 48, (void*)32);



    }
    void DrawTriangles() const
    {

        PopulateBuffers();
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(verticies.size()));

    }
    void DrawLines() const
    {

        PopulateBuffers();
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(verticies.size()));

    }

};

struct MeshGroup
{
    GLuint VAO;
	std::vector<Mesh> meshes;
	int rootIndex;
	std::string name;
	unsigned tag = 0;

    void DrawTriangles()
    {
        glBindVertexArray(VAO);
        for (auto mesh : meshes)
        {
            mesh.DrawTriangles();
        }
        glBindVertexArray(0);
    }
    void DrawLines()
    {
        glBindVertexArray(VAO);
        for (auto mesh : meshes)
        {
            mesh.DrawLines();
        }
        glBindVertexArray(0);
    }
};

class FBXImporter
{
public:
	static std::vector<MeshGroup*> Load(const char* file_path) noexcept;
	static FbxScene* ImportFbx(FbxManager* p_manager, const char* file_path) noexcept;
private:
	static std::vector<MeshGroup*> Parse(FbxNode* p_root) noexcept;
	static int ParseNode(FbxNode* p_node, int parent, std::vector<Mesh>& polygons) noexcept;
	static std::vector<Vertex> GetAttribute(const FbxMesh* p_mesh) noexcept;
};