/*
 *	Author		: Jina Hyun
 *	Date		: 09/16/22
 *	File Name	: Input.cpp
 *	Desc		: Input system
 */

#include "Input.h"

#include <iostream>
#include <ostream>	// std::ostream
#include <GLFW/glfw3.h>	// glfw functions

const glm::ivec2 Input::s_m_windowSize = glm::ivec2(1200, 900);
bool Input::s_m_isMouseDown[2] = { false, false };
ModKey Input::s_m_mouseMod = ModKey::None;
glm::ivec2 Input::s_m_cursor = glm::ivec2(0, 0);
glm::ivec2 Input::s_m_cursorDir = glm::ivec2(0);

std::ostream& operator<<(std::ostream& os, const glm::ivec2& v)
{
	return os << "(" << v.x << ", " << v.y << ")";
}

void Input::KeyboardCallback(void* p_window, int key, int, int action, int) noexcept
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(static_cast<GLFWwindow*>(p_window), GLFW_TRUE);
	}
}

void Input::CursorPosCallback(void*, double x_pos, double y_pos) noexcept
{
	const glm::ivec2 pos = glm::ivec2(x_pos, s_m_windowSize.y - y_pos);
	if (s_m_isMouseDown[0] || s_m_isMouseDown[1])
		s_m_cursorDir = pos - s_m_cursor;
	s_m_cursor = pos;
}

void Input::MouseButtonCallback(void*, int button, int action, int mod) noexcept
{
	if (action == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			s_m_isMouseDown[0] = false;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
			s_m_isMouseDown[1] = false;
		s_m_mouseMod = ModKey::None;
		s_m_cursorDir = glm::vec2(0);
	}
	else
	{
		if (mod == GLFW_MOD_SHIFT)
			s_m_mouseMod = ModKey::Shift;
		else if (mod == GLFW_MOD_CONTROL)
			s_m_mouseMod = ModKey::Control;
		else if (mod == GLFW_MOD_ALT)
			s_m_mouseMod = ModKey::Alt;

		if (button == GLFW_MOUSE_BUTTON_LEFT)
			s_m_isMouseDown[0] = true;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
			s_m_isMouseDown[1] = true;
	}
}

void Input::ScrollCallback(void*, double, double) noexcept
{
}

glm::ivec2 Input::GetMouseMovingDirection(MouseButton button) noexcept
{
	return s_m_isMouseDown[static_cast<int>(button)] ? s_m_cursorDir : glm::ivec2(0);
}

ModKey Input::GetMouseMod() noexcept
{
	return s_m_mouseMod;
}
