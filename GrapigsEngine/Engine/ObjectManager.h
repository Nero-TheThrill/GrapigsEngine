#pragma once
#include <vector>
#include <string>




struct Object
{
    //Transform transform;
    unsigned shader = 0;
    unsigned mesh = 0;
    unsigned tag = 0;
    std::string name = "";
};


class ObjectManager
{
public:
    ObjectManager();
    ~ObjectManager();
    Object* GetObjectByName(std::string target_name);
    std::vector<Object*> GetObjectByTag(unsigned target_tag);
    Object* CreateObject(unsigned obj_tag, std::string obj_name, unsigned mesh_tag, unsigned shader_tag);
    //void AddObject(Object* obj);
    void DeleteObject(Object* obj);
    
private:
    std::vector<Object*> obj_storage;
};

