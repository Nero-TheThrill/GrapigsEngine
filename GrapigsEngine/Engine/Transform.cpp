#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>

void Transform::Reset() noexcept
{
    m_position = glm::vec3{ 0 };
    m_rotation = glm::vec3{ 0 };
    m_scaling = glm::vec3{ 1 };
    translate = glm::mat4{ 1 };
    rotate = glm::mat4{ 1 };
    scale = glm::mat4{ 1 };
}

void Transform::Translate(glm::vec3 input)
{
    m_position = input;
    translate = glm::translate(glm::mat4{1}, m_position);
    transform = translate * rotate * scale;
}

void Transform::Rotate(float degree, glm::vec3 v)
{
    m_rotation = v * degree;
    rotate = glm::rotate(glm::mat4{ 1 }, glm::radians(degree), v);
    transform = translate * rotate * scale;
}

void Transform::Scale(glm::vec3 input)
{
    m_scaling = input;
    scale = glm::scale(glm::mat4{ 1 }, m_scaling);
    transform = translate * rotate * scale;
}

void Transform::Scale(float s)
{
    m_scaling = glm::vec3{ s };
    scale = glm::scale(glm::mat4{ 1 }, m_scaling);
    transform = translate * rotate * scale;
}

void Transform::Rotate(float x, float y, float z)
{
    m_rotation.x = x;
    m_rotation.y = y;
    m_rotation.z = z;
    rotate = glm::rotate(glm::mat4{ 1 }, glm::radians(x), glm::vec3{1, 0, 0});
    rotate = glm::rotate(rotate, glm::radians(y), glm::vec3{0, 1, 0});
    rotate = glm::rotate(rotate, glm::radians(z), glm::vec3{0, 0, 1});
    transform = translate * rotate * scale;
}

const glm::vec3& Transform::GetPosition() const noexcept
{
    return m_position;
}

const glm::vec3& Transform::GetRotation() const noexcept
{
    return m_rotation;
}

const glm::vec3& Transform::GetScaling() const noexcept
{
    return m_scaling;
}

const glm::mat4& Transform::GetTransformMatrix() const
{
    return transform;
}
