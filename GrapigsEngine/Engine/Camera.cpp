/*
 *	Author		: Jina Hyun
 *	Date		: 09/16/22
 *	File Name	: Camera.h
 *	Desc		: Camera functions
 */
#include "Camera.h"

#include <iostream>
#include <gl/glew.h>	// gl functions for camera buffer
#include <glm/gtc/matrix_transform.hpp>	// matrix calculation

#include "Input.h"

 /* Camera - start -------------------------------------------------------------------------------*/

Camera::Camera() noexcept
	:
	m_isUpdated(true),
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

Camera::Camera(const glm::vec3& position, const glm::vec3& right_vector, const glm::vec3& up_vector,
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

void Camera::Set(const glm::vec3& eye, const glm::vec3& look, const glm::vec3& world_up) noexcept
{
	m_eye = eye;
	m_back = glm::normalize(-look);
	m_right = glm::normalize(glm::cross(m_back, world_up));
	m_up = glm::normalize(glm::cross(m_right, m_back));
	UpdateMatrix();
}

void Camera::Set(const glm::vec3& eye) noexcept
{
	m_eye = eye;
	m_back = glm::normalize(-eye);
	m_right = glm::normalize(glm::cross(m_back, glm::vec3{ 0, 1, 0 }));
	m_up = glm::normalize(glm::cross(m_right, m_back));
	UpdateMatrix();
}

void Camera::Set(const glm::mat4& view) noexcept
{
	m_right = view[0];
	m_up = view[1];
	m_back = -view[2];
	const glm::mat3 mat{ view };
	const glm::mat3 inv = glm::inverse(mat); // TODO: fix
	m_eye = -inv * glm::vec3{ -view[3][0], -view[3][1], view[3][2]};
	UpdateMatrix();
	m_isUpdated = false;
}

void Camera::Reset() noexcept
{
	Set(glm::vec3{ 0, 0, 1.3 });
	UpdateMatrix();
}

bool Camera::IsUpdated() noexcept
{
	if(m_isUpdated)
	{
		m_isUpdated = false;
		return true;
	}
	return false;
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

float Camera::FOV() const noexcept
{
	return m_fov;
}

std::pair<float, float> Camera::Plane() const noexcept
{
	return std::make_pair(m_near, m_far);
}

void Camera::Forward(float z_axis_distance) noexcept
{
	m_eye = m_eye - z_axis_distance * m_back;
	if (const auto dist = glm::length(m_eye); dist > 8)
		m_eye = glm::normalize(m_eye) * 8.f;
	else if (dist < 1)
		m_eye = glm::normalize(m_eye);
	UpdateMatrix();
}

void Camera::OrbitMouse(float x, float y, float speed) noexcept
{
	const float dist = glm::length(m_eye);
	if(abs(x) > std::numeric_limits<float>::epsilon())
		m_eye = glm::normalize(m_eye + x * speed * m_right) * dist;
	if(abs(y) > std::numeric_limits<float>::epsilon())
	{
		const auto eye = glm::normalize(m_eye + y * speed * m_up) * dist;
		if(abs(eye.x - m_eye.x) < std::numeric_limits<float>::epsilon()
			|| abs(eye.y - m_eye.y) < std::numeric_limits<float>::epsilon()
			|| abs(eye.z - m_eye.z) < std::numeric_limits<float>::epsilon())
		{
			m_eye = glm::normalize(eye + y * m_up * speed) * dist;
		}
		else
			m_eye = eye;
	}
	m_back = -glm::normalize(m_eye);
	m_right = glm::cross(m_back, glm::vec3{ 0, 1, 0 });
	m_up = glm::cross(m_right, m_back);
	UpdateMatrix();
}

void Camera::TranslateMouse(float x, float y, float speed) noexcept
{
	if (abs(x) > std::numeric_limits<float>::epsilon())
		m_eye += x * glm::cross(m_back, glm::vec3{0, 1, 0}) * speed;
	if (abs(y) > std::numeric_limits<float>::epsilon())
		m_eye += y * glm::cross(glm::vec3{1, 0, 0}, m_back) * speed;
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

void Camera::RotateMouse(float x, float y, float speed) noexcept
{
	if (abs(x) > std::numeric_limits<float>::epsilon())
	{
		const glm::mat4& rotation = glm::rotate(glm::mat4(1), glm::radians(x * speed), glm::vec3{ 0, 1, 0 });
		m_up = rotation * glm::vec4(m_up, 0);
		m_right = rotation * glm::vec4(m_right, 0);
		m_back = rotation * glm::vec4(m_back, 0);
	}
	if (abs(y) > std::numeric_limits<float>::epsilon())
	{
		const glm::mat4& rotation = glm::rotate(glm::mat4(1), glm::radians(y * speed), glm::vec3{ 1, 0, 0 });
		m_up = rotation * glm::vec4(m_up, 0);
		m_right = rotation * glm::vec4(m_right, 0);
		m_back = rotation * glm::vec4(m_back, 0);
	}
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
	m_isUpdated = true;
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
		glNamedBufferStorage(s_m_handle, sizeof(glm::mat4) * 3 + sizeof(glm::vec3) + sizeof(float) * 2, nullptr, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, s_m_handle);
	}
}

void CameraBuffer::UpdateMainCamera() noexcept
{
	auto cursor_dir = Input::GetMouseMovingDirection(MouseButton::Right);
	if (cursor_dir.x != 0 || cursor_dir.y != 0)
	{
		const glm::vec2 mouse = glm::vec2{ cursor_dir.x, cursor_dir.y };
		switch (Input::GetModifier())
		{
		case Modifier::None:
			s_m_camera->OrbitMouse(-mouse.x, -mouse.y, 0.005f);
		break;
		case Modifier::Control:
			{
				const float sign = cursor_dir.y > 0 ? -1.f : 1.f;
				s_m_camera->Forward(sign * 0.02f);
			}
		break;
		default: break;
		}
	}



	if(const int scroll = Input::GetMouseScroll(); scroll)
	{
		s_m_camera->Forward(static_cast<float>(scroll) * 0.1f);
	}

	cursor_dir = Input::GetMouseMovingDirection(MouseButton::Middle);
	if (cursor_dir.x != 0 || cursor_dir.y != 0)
	{
		const glm::vec2 mouse = glm::vec2{ cursor_dir.x, cursor_dir.y };
		switch (Input::GetModifier())
		{
		case Modifier::None:
			s_m_camera->RotateMouse(-mouse.x, mouse.y, 0.05f);
			break;
		case Modifier::Control:
			s_m_camera->TranslateMouse(-mouse.x, -mouse.y, 0.002f);
			break;
		default: break;
		}
	}


	if (Input::IsKeyReleased(Keyboard::R))
		s_m_camera->Reset();
}

Camera* CameraBuffer::GetMainCamera() noexcept
{
	return s_m_camera;
}

void CameraBuffer::Bind() noexcept
{
	glNamedBufferSubData(s_m_handle, 0, sizeof(glm::mat4), &s_m_camera->m_worldToCamera[0][0]);
	glNamedBufferSubData(s_m_handle, sizeof(glm::mat4), sizeof(glm::mat4), &s_m_camera->m_cameraToNDC[0][0]);
	glNamedBufferSubData(s_m_handle, sizeof(glm::mat4) * 2, sizeof(glm::mat4), &s_m_camera->m_worldToNDC[0][0]);
	glNamedBufferSubData(s_m_handle, sizeof(glm::mat4) * 3, sizeof(glm::vec3), &s_m_camera->Eye()[0]);
	glNamedBufferSubData(s_m_handle, sizeof(glm::mat4) * 3 + sizeof(glm::vec3), sizeof(float), &s_m_camera->m_near);
	glNamedBufferSubData(s_m_handle, sizeof(glm::mat4) * 3 + sizeof(glm::vec3) + sizeof(float), sizeof(float), &s_m_camera->m_far);
	glBindBuffer(GL_UNIFORM_BUFFER, s_m_handle);
}

glm::vec3 CameraBuffer::GetMouseRay() noexcept
{
	const glm::mat4& invProj = glm::inverse(s_m_camera->GetCameraToNDCMatrix());
	const glm::mat4& invView = glm::inverse(s_m_camera->GetWorldToCameraMatrix());
	const glm::vec3 pos = invView * (invProj * glm::vec4(Input::GetNormalizedMousePos(), 1));	
	return glm::normalize(pos);
}

void CameraBuffer::UpdateMatrix() noexcept
{
	s_m_camera->UpdateMatrix();
}

/* CameraBuffer - end ---------------------------------------------------------------------------*/
