/*
 *	Author		: Jina Hyun
 *	Date		: 09/16/22
 *	File Name	: Camera.h
 *	Desc		: Camera functions
 */
#include "Camera.h"

#include <ostream>	// std::ostream
#include <gl/glew.h>	// gl functions for camera buffer
#include <glm/gtc/matrix_transform.hpp>	// matrix calculation

#include "Input.h"

std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
{
	return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

 /* Camera - start -------------------------------------------------------------------------------*/

Camera::Camera() noexcept
	:
	m_near(0.1f), m_far(100), m_fov(90),
	m_eye(0, 0, 3),
	m_right(1, 0, 0),
	m_up(0, 1, 0),
	m_back(0, 0, -1),
	m_worldToCamera(1),
	m_cameraToNDC(1),
	m_worldToNDC(1)
{
	UpdateMatrix();
}

void Camera::SetCamera(const glm::vec3& position, const glm::vec3& right_vector, const glm::vec3& up_vector,
	const glm::vec3& back_vector, float near_plane, float far_plane, float field_of_view) noexcept
{
	m_eye = position;
	m_right = right_vector;
	m_up = up_vector;
	m_back = back_vector;
	m_near = near_plane;
	m_far = far_plane;
	m_fov = field_of_view;
	UpdateMatrix();
}

const glm::mat4& Camera::GetWorldToCameraMatrix() const noexcept
{
	return m_worldToCamera;
}

const glm::mat4& Camera::GetCameraToNDCMatrix() const noexcept
{
	return  m_cameraToNDC;
}

const glm::vec3& Camera::Eye() const noexcept
{
	return m_eye;
}

const glm::vec3& Camera::Back() const noexcept
{
	return m_back;
}

void Camera::Forward(float z_axis_distance) noexcept
{
	m_eye = m_eye - z_axis_distance * m_back;
	UpdateMatrix();
}

void Camera::Upward(float y_axis_distance) noexcept
{
	m_eye = m_eye - y_axis_distance * m_up;
	UpdateMatrix();
}

void Camera::Sideward(float x_axis_distance) noexcept
{
	m_eye = m_eye - x_axis_distance * m_right;
	UpdateMatrix();
}

void Camera::Pitch(float pitch_angle) noexcept
{
	const glm::mat4& rotation = glm::rotate(glm::mat4(1), glm::radians(pitch_angle), m_right);
	m_up = rotation * glm::vec4(m_up, 0);
	m_back = rotation * glm::vec4(m_back, 0);
	UpdateMatrix();
}

void Camera::Yaw(float yaw_angle) noexcept
{
	const glm::mat4& rotation = glm::rotate(glm::mat4(1), glm::radians(yaw_angle), m_up);
	m_right = rotation * glm::vec4(m_right, 0);
	m_back = rotation * glm::vec4(m_back, 0);
	UpdateMatrix();
}

void Camera::Roll(float roll_angle) noexcept
{
	const glm::mat4& rotation = glm::rotate(glm::mat4(1), glm::radians(roll_angle), m_back);
	m_up = rotation * glm::vec4(m_up, 0);
	m_right = rotation * glm::vec4(m_right, 0);
	UpdateMatrix();
}

void Camera::UpdateMatrix() noexcept
{
	m_worldToCamera = glm::lookAt(m_eye, m_eye + m_back, m_up);
	m_cameraToNDC = glm::perspective(glm::radians(m_fov), CameraBuffer::s_m_aspectRatio, m_near, m_far);
	m_worldToNDC = m_cameraToNDC * m_worldToCamera;
}

/* Camera - end ---------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* CameraBuffer - start -------------------------------------------------------------------------*/

unsigned CameraBuffer::s_m_handle = 0;
float CameraBuffer::s_m_aspectRatio = 1200.f / 900.f;
Camera* CameraBuffer::s_m_camera = nullptr;

void CameraBuffer::Clear() noexcept
{
	delete s_m_camera;
	s_m_camera = nullptr;
	glDeleteBuffers(1, &s_m_handle);
	s_m_handle = 0;
}

void CameraBuffer::SetMainCamera(Camera* p_camera) noexcept
{
	s_m_camera = p_camera;
	if (s_m_handle < 1)
	{
		glCreateBuffers(1, &s_m_handle);
		glNamedBufferStorage(s_m_handle, sizeof(glm::mat4), nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, s_m_handle);
	}
}

void CameraBuffer::UpdateMainCamera() noexcept
{
	const auto& cursor_dir = Input::GetMouseMovingDirection(MouseButton::Right);
	if (cursor_dir.x != 0 || cursor_dir.y != 0)
	{
		switch (Input::GetMouseMod())
		{
		case ModKey::None:
		{
			// TODO: Orbit camera
		}
		break;
		case ModKey::Shift:
		{
			s_m_camera->Upward(static_cast<float>(cursor_dir.y) * 0.01f);
			s_m_camera->Sideward(static_cast<float>(cursor_dir.x) * 0.01f);
		}
		break;
		case ModKey::Control:
		{
			const float speed = std::sqrt(static_cast<float>(cursor_dir.x * cursor_dir.x + cursor_dir.y * cursor_dir.y));
			const float sign = cursor_dir.y > 0 ? -1.f : 1.f;
			s_m_camera->Forward(speed * sign * 0.001f);
		}
		break;
		case ModKey::Alt: break;
		}
	}
}

const Camera* CameraBuffer::GetMainCamera() noexcept
{
	return s_m_camera;
}

void CameraBuffer::Bind() noexcept
{
	glNamedBufferSubData(s_m_handle, 0, sizeof(glm::mat4), &s_m_camera->m_worldToNDC[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, s_m_handle);
}

/* CameraBuffer - end ---------------------------------------------------------------------------*/
