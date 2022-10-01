/*
 *	Author		: Jiwoo Choi
 *	Date		: 09/26/22
 *	File Name	: ResourceManager.h
 *	Desc		: Manage shader, object
 */
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class Transform 
{
public:
    void Init();
    glm::mat4 GetTransformMatrix() const;
    ~Transform();
    void Translate(glm::vec3 input);
    void Rotate(float degree, glm::vec3 v);
    void Scale(glm::vec3 input);
    void Move(glm::vec3 input);

    glm::vec3 position = glm::vec3(0.0f);
    glm::mat4 transform = glm::mat4(1.0f);
    glm::mat4 translate = glm::mat4(1.0f), rotate = glm::mat4(1.0f), scale = glm::mat4(1.0f);
};

