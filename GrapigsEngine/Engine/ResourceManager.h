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
#include "FBXImporter.h"
#include "ShaderSet.h"

struct Object
{
    Transform transform;
    GLuint shader = 0;
    MeshGroup* gmesh = 0;
    unsigned tag = 0;
    std::string name;

    void DrawTriangles()
    {
        glUseProgram(shader);
        Set(shader, "model", transform.GetTransformMatrix());
        gmesh->DrawTriangles();
        glUseProgram(0);
    };
    void DrawLines()
    {
        glUseProgram(shader);
        Set(shader, "model", transform.GetTransformMatrix());
        gmesh->DrawLines();
        glUseProgram(0);
    };
};


class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    void LoadMesh(const std::string& path);
    MeshGroup* GetMeshByName(const std::string& target_name);
    MeshGroup* GetMeshByTag(const unsigned& target_tag);

    Object* CreateObject(unsigned obj_tag, std::string obj_name, unsigned mesh_tag, unsigned shader_tag);
    Object* GetObjectByName(std::string target_name);
    std::vector<Object*> GetObjectByTag(const unsigned& target_tag);
    //void AddObject(Object* obj);
    void DeleteObject(Object* obj);


    void CompileShader(unsigned tag, const std::string& vert_path, const std::string& frag_path);
    GLuint GetShaderByTag(const unsigned& target_tag);
    

private:
    std::vector<Object*> obj_storage;
    std::vector<MeshGroup*> gmesh_storage;
    std::vector<std::pair<unsigned, GLuint>> shader_storage;
    
    GLuint LoadVertexShader(const std::string& path);
    GLuint LoadFragmentShader(const std::string& path);

    FBXImporter importer;


    std::string log_string;

};

