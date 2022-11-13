/*
 *	Author		: Jina Hyun
 *	Date		: 09/16/22
 *	File Name	: Camera.h
 *	Desc		: Camera functions
 */
#pragma once
#include <glm/glm.hpp>	// glm

class Camera
{
	friend class CameraBuffer;
public:
	Camera() noexcept;
	Camera(const glm::vec3& position, const glm::vec3& right_vector, const glm::vec3& up_vector, const glm::vec3& back_vector, float near_plane, float far_plane, float field_of_view) noexcept;
	void Set(const glm::vec3& eye, const glm::vec3& look, const glm::vec3& world_up) noexcept;
	void Set(const glm::vec3& eye) noexcept;
	void Set(const glm::mat4& view) noexcept;
	void Reset() noexcept;

	[[nodiscard]] bool IsUpdated() noexcept;

	[[nodiscard]] const glm::mat4& GetWorldToCameraMatrix() const noexcept;
	[[nodiscard]] const glm::mat4& GetCameraToNDCMatrix() const noexcept;

	[[nodiscard]] const glm::vec3& Eye() const noexcept;
	[[nodiscard]] const glm::vec3& Back() const noexcept;
	[[nodiscard]] float FOV() const noexcept;
	[[nodiscard]] std::pair<float, float> Plane() const noexcept;

	void Forward(float z_axis_distance) noexcept;
	void OrbitMouse(float x, float y, float speed) noexcept;
	void TranslateMouse(float x, float y, float speed) noexcept;

	void Pitch(float pitch_angle) noexcept;
	void Roll(float roll_angle) noexcept;
	void Yaw(float yaw_angle) noexcept;
	void RotateMouse(float x, float y, float speed) noexcept;
private:
	void UpdateMatrix() noexcept;

	bool m_isUpdated;
	float m_near, m_far, m_fov;
	glm::vec3 m_eye, m_right, m_up, m_back;
	glm::mat4 m_worldToCamera, m_cameraToNDC, m_worldToNDC;
};

class CameraBuffer
{
public:
	static void Clear() noexcept;
	static void SetMainCamera(Camera* p_camera) noexcept;
	static void UpdateMainCamera() noexcept;
	static Camera* GetMainCamera() noexcept;
	static void Bind() noexcept;
	static glm::vec3 GetMouseRay() noexcept;
	static void UpdateMatrix() noexcept;
	static float s_m_aspectRatio;
	static Camera* s_m_camera;
private:
	static unsigned s_m_handle;
};
