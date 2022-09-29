/*
 *	Author		: Jiwoo Choi
 *	Date		: 09/06/22
 *	File Name	: ResourceManager.cpp
 *	Desc		: Manage shader, object
 */
#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

#include "FBXImporter.h"    // FBX Importer


ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}
void ResourceManager::LoadMesh(const std::string& path)
{
    auto dummies = FBXImporter::Load(path.c_str());

    //Mesh* mesh = new Mesh();
    //glGenVertexArrays(1, &mesh->VAO);
    //glBindVertexArray(mesh->VAO);
    //glGenBuffers(NUM_VBO, mesh->VBO);


    //glBindVertexArray(0);

    for(auto& d : dummies)
        delete d;
}

Mesh* ResourceManager::GetMeshByName(const std::string& target_name)
{
    for(auto mesh : mesh_storage)
    {
        if(mesh->name== target_name)
        {
            return mesh;
        }
    }
    std::cout << "Mesh Not Found. Name : " << target_name << std::endl;
    return nullptr;
}

Mesh* ResourceManager::GetMeshByTag(const unsigned& target_tag)
{
    for (auto mesh : mesh_storage)
    {
        if (mesh->tag == target_tag)
        {
            return mesh;
        }
    }
    std::cout << "Mesh Not Found. Tag : " << target_tag << std::endl;
    return nullptr;
}

Object* ResourceManager::GetObjectByName(std::string target_name)
{
    std::vector<Object*> result;

    for (auto obj : obj_storage)
    {
        if (obj->name == target_name)
        {
            return obj;
        }
    }
    std::cout << "Object Not Found. Name : " << target_name << std::endl;
    return nullptr;
}

std::vector<Object*> ResourceManager::GetObjectByTag(const unsigned& target_tag)
{
    std::vector<Object*> result;

    for (auto obj : obj_storage)
    {
        if (obj->tag == target_tag)
        {
            result.push_back(obj);
        }
    }
    if(result.size()==0)
    {
        std::cout << "Object Not Found. Tag : " << target_tag << std::endl;
    }
    return result;
}

Object* ResourceManager::CreateObject(unsigned obj_tag, std::string obj_name, unsigned mesh_tag, unsigned shader_tag)
{
    Object* new_obj = new Object();

    new_obj->mesh = GetMeshByTag(mesh_tag);
    new_obj->shader = shader_tag;
    new_obj->tag = obj_tag;
    new_obj->name = obj_name;
    //new_obj->transform = ;

    obj_storage.push_back(new_obj);

    return new_obj;
}

void ResourceManager::DeleteObject(Object* obj)
{
    auto target_obj = std::find(obj_storage.begin(), obj_storage.end(), obj);

    if (target_obj != obj_storage.end())
    {
        obj_storage.erase(target_obj);
    }
}

void ResourceManager::CompileShader(unsigned tag, const std::string& vert_path, const std::string& frag_path)
{
    GLuint program_handle;

    program_handle = glCreateProgram();
    if (0 == program_handle) {
        log_string = "Cannot create program handle";
        return;
    }


    glAttachShader(program_handle, LoadVertexShader(vert_path));
    glAttachShader(program_handle, LoadFragmentShader(frag_path));

    // verify link status
    glLinkProgram(program_handle);
    GLint linked = 0;
    glGetProgramiv(program_handle, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE)
    {
        GLint log_length = 0;
        glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> error_log(log_length);
        glGetProgramInfoLog(program_handle, log_length, nullptr, error_log.data());
        glDeleteProgram(program_handle);
        std::cout << ("[ShaderProgram]: Link Failure\n" + std::string(error_log.begin(), error_log.end()));
        return;
    }

    // Check validate
    glValidateProgram(program_handle);
    GLint is_validate = 0;
    glGetProgramiv(program_handle, GL_VALIDATE_STATUS, &is_validate);
    if (is_validate == GL_FALSE)
    {
        GLint log_length = 0;
        glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> error_log(log_length);
        glGetProgramInfoLog(program_handle, log_length, nullptr, error_log.data());
        glDeleteProgram(program_handle);
        std::cout << ("[ShaderProgram]: Validate Failure\n" + std::string(error_log.begin(), error_log.end()));
        return;
    }

    shader_storage.emplace_back(std::pair<unsigned, GLuint>(tag, program_handle));
}

GLuint ResourceManager::GetShaderByTag(const unsigned& target_tag)
{
    for (auto shader : shader_storage)
    {
        if (shader.first == target_tag)
        {
            return shader.second;
        }
    }
    std::cout << "Shader Not Found. Tag : " << target_tag << std::endl;
    return 0;
}

GLuint ResourceManager::LoadVertexShader(const std::string& path)
{
    std::ifstream shader_file(path, std::ifstream::in);
    if (!shader_file) {
        log_string = "Error opening file " + path;
        std::cout << log_string << std::endl;
        return 0;
    }
    std::stringstream buffer;
    buffer << shader_file.rdbuf();
    shader_file.close();


    std::string stringbuffer = buffer.str();

    GLuint vertexshader_handle;
    GLint vertexshader_result;
    char const* vertexshader[] = { stringbuffer.c_str() };


    vertexshader_handle = glCreateShader(GL_VERTEX_SHADER);


    glShaderSource(vertexshader_handle, 1, vertexshader, NULL);
    glCompileShader(vertexshader_handle);
    glGetShaderiv(vertexshader_handle, GL_COMPILE_STATUS, &vertexshader_result);
    if (!vertexshader_result)
    {
        log_string = " vertex shader compilation failed\n";
        GLint log_len;
        glGetShaderiv(vertexshader_handle, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char* log = new char[log_len];
            GLsizei written_log_len;
            glGetShaderInfoLog(vertexshader_handle, log_len, &written_log_len, log);
            log_string += log;
            std::cout << log_string << std::endl;
            delete[] log;
        }
    }


    return vertexshader_handle;
}

GLuint ResourceManager::LoadFragmentShader(const std::string& path)
{
    std::ifstream shader_file(path, std::ifstream::in);
    if (!shader_file) {
        log_string = "Error opening file " + path;
        std::cout << log_string << std::endl;
        return 0;
    }
    std::stringstream buffer;
    buffer << shader_file.rdbuf();
    shader_file.close();


    std::string stringbuffer = buffer.str();


    GLuint fragmentshader_handle;
    GLint  fragmentshader_result;

    char const* fragmentshader[] = { stringbuffer.c_str() };
    fragmentshader_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader_handle, 1, fragmentshader, NULL);
    glCompileShader(fragmentshader_handle);
    glGetShaderiv(fragmentshader_handle, GL_COMPILE_STATUS, &fragmentshader_result);
    if (!fragmentshader_result)
    {
        log_string = " fragment shader compilation failed\n";
        GLint log_len;
        glGetShaderiv(fragmentshader_handle, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 0) {
            char* log = new char[log_len];
            GLsizei written_log_len;
            glGetShaderInfoLog(fragmentshader_handle, log_len, &written_log_len, log);
            log_string += log;
            std::cout << log_string << std::endl;
            delete[] log;
        }
    }

    return fragmentshader_handle;
}
