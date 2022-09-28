/*
 *	Author		: Jiwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.h
 *	Desc		: Manage shader, object
 */
#pragma once
#include <vector>
#include <string>
#include <gl/glew.h>	
#include <GLFW/glfw3.h>

#include "Transform.h"
#include "glm/glm.hpp"

#define NUM_VBO 3

struct Mesh
{
    GLuint VAO;
    GLuint VBO[NUM_VBO];
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned> indices;
    std::string name;
    unsigned tag = 0;
    void PopulateBuffers() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        //glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), normals.data(), GL_STATIC_DRAW);
        //glEnableVertexAttribArray(1);
        //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        //glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords[0]) * texcoords.size(), texcoords.data(), GL_STATIC_DRAW);
        //glEnableVertexAttribArray(2);
        //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    void Draw() const
    {
        glBindVertexArray(VAO);
        PopulateBuffers();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, indices.data());
        glBindVertexArray(0);
    }

};
struct Object
{
    Transform transform;
    GLuint shader = 0;
    Mesh* mesh = 0;
    unsigned tag = 0;
    std::string name;

    void Draw()
    {
        glUseProgram(shader);
        mesh->Draw();
        glUseProgram(0);
    }
};


class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    void LoadMesh(const std::string& path);
    Mesh* GetMeshByName(const std::string& target_name);
    Mesh* GetMeshByTag(const unsigned& target_tag);

    Object* CreateObject(unsigned obj_tag, std::string obj_name, unsigned mesh_tag, unsigned shader_tag);
    Object* GetObjectByName(std::string target_name);
    std::vector<Object*> GetObjectByTag(const unsigned& target_tag);
    //void AddObject(Object* obj);
    void DeleteObject(Object* obj);


    void CompileShader(unsigned tag, const std::string& vert_path, const std::string& frag_path);
    GLuint GetShaderByTag(const unsigned& target_tag);
    
    std::vector<Mesh*> mesh_storage;
private:
    std::vector<Object*> obj_storage;
    std::vector<std::pair<unsigned, GLuint>> shader_storage;
    
    GLuint LoadVertexShader(const std::string& path);
    GLuint LoadFragmentShader(const std::string& path);



    std::string log_string;

};

