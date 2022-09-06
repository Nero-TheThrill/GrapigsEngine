#include "ObjectManager.h"

ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
}

Object* ObjectManager::GetObjectByName(std::string target_name)
{
    std::vector<Object*> result;

    for (auto obj : obj_storage)
    {
        if (obj->name == target_name)
        {
            return obj;
        }
    }

    return nullptr;
}

std::vector<Object*> ObjectManager::GetObjectByTag(unsigned target_tag)
{
    std::vector<Object*> result;

    for (auto obj : obj_storage)
    {
        if (obj->tag == target_tag)
        {
            result.push_back(obj);
        }
    }

    return result;
}

Object* ObjectManager::CreateObject(unsigned obj_tag, std::string obj_name, unsigned mesh_tag, unsigned shader_tag)
{
    Object* new_obj = new Object();

    new_obj->mesh = mesh_tag;
    new_obj->shader = shader_tag;
    new_obj->tag = obj_tag;
    new_obj->name = obj_name;
    //new_obj->transform = ;

    obj_storage.push_back(new_obj);

    return new_obj;
}

void ObjectManager::DeleteObject(Object* obj)
{
    auto target_obj = std::find(obj_storage.begin(), obj_storage.end(), obj);

    if (target_obj != obj_storage.end())
    {
        obj_storage.erase(target_obj);
    }
}
