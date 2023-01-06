/*
 *	Author		: Jinwoo Choi, Jina Hyun
 *	Date		: 09/26/22
 *	File Name	: Transform.h
 *	Desc		: transform
 */
#pragma once
#include <glm/glm.hpp>
class Transform 
{
public:
    Transform() = default;
    ~Transform() = default;
    void Reset() noexcept;
    void Translate(glm::vec3 input);
    void Rotate(float degree, glm::vec3 v);
    void Scale(glm::vec3 input);
    void Scale(float s);
    void Rotate(float x, float y, float z);

    const glm::vec3& GetPosition() const noexcept;
    const glm::vec3& GetRotation() const noexcept;
    const glm::vec3& GetScaling() const noexcept;

    [[nodiscard]] const glm::mat4& GetTransformMatrix() const;
private:
    glm::vec3 m_position{ 0 };
    glm::vec3 m_rotation{ 0 };
    glm::vec3 m_scaling{ 1 };
    glm::mat4 translate = glm::mat4(1.0f), rotate = glm::mat4(1.0f), scale = glm::mat4(1.0f);
    glm::mat4 transform = glm::mat4(1.0f);
};

