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



struct Object
{
    //Transform transform;
    unsigned shader = 0;
    unsigned mesh = 0;
    unsigned tag = 0;
    std::string name = "";
};


class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();
    Object* GetObjectByName(std::string target_name);
    std::vector<Object*> GetObjectByTag(unsigned target_tag);
    Object* CreateObject(unsigned obj_tag, std::string obj_name, unsigned mesh_tag, unsigned shader_tag);
    //void AddObject(Object* obj);
    void DeleteObject(Object* obj);
    void CompileShader(unsigned tag, const std::string& vert_path, const std::string& frag_path);
    
private:
    std::vector<Object*> obj_storage;
    std::vector<std::pair<unsigned, GLuint>> shader_storage;
    
    GLuint LoadVertexShader(const std::string& path);
    GLuint LoadFragmentShader(const std::string& path);
    std::string log_string;

};

