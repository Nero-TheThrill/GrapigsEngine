/*
 *	Author		: Jina Hyun
 *	Date		: 09/16/22
 *	File Name	: Input.cpp
 *	Desc		: Input system
 */
#include "Input.h"

#include <iostream>	// std::cout
#include <GLFW/glfw3.h>	// glfw functions

glm::ivec2 Input::s_m_windowSize = glm::ivec2(1200, 900);
bool Input::s_m_isMouseDown[2] = { false, false };
Modifier Input::s_m_modifier = Modifier::None;
glm::ivec2 Input::s_m_cursor = glm::ivec2(0, 0);
glm::ivec2 Input::s_m_cursorDir = glm::ivec2(0);
glm::vec3 Input::s_m_ray = glm::vec3(0, 0, 1);
std::bitset<static_cast<size_t>(Keyboard::Unknown) + 1> Input::s_m_keyPress;
std::bitset<static_cast<size_t>(Keyboard::Unknown) + 1> Input::s_m_keyRelease;
std::vector<std::filesystem::path> Input::s_m_droppedPath;

namespace
{
	Keyboard ToKeyboard(int key)
	{
		switch (key)
		{
		case GLFW_KEY_SPACE: return Keyboard::Space;
		case GLFW_KEY_0: return Keyboard::Num0;		case GLFW_KEY_1: return Keyboard::Num1;		case GLFW_KEY_2: return Keyboard::Num2;
		case GLFW_KEY_3: return Keyboard::Num3;		case GLFW_KEY_4: return Keyboard::Num4;		case GLFW_KEY_5: return Keyboard::Num5;
		case GLFW_KEY_6: return Keyboard::Num6;		case GLFW_KEY_7: return Keyboard::Num7;		case GLFW_KEY_8: return Keyboard::Num8;
		case GLFW_KEY_9: return Keyboard::Num9;
		case GLFW_KEY_A: return Keyboard::A;		case GLFW_KEY_B: return Keyboard::B;		case GLFW_KEY_C: return Keyboard::C;
		case GLFW_KEY_D: return Keyboard::D;		case GLFW_KEY_E: return Keyboard::E;		case GLFW_KEY_F: return Keyboard::F;
		case GLFW_KEY_G: return Keyboard::G;		case GLFW_KEY_H: return Keyboard::H;		case GLFW_KEY_I: return Keyboard::I;
		case GLFW_KEY_J: return Keyboard::J;		case GLFW_KEY_K: return Keyboard::K;		case GLFW_KEY_L: return Keyboard::L;
		case GLFW_KEY_M: return Keyboard::M;		case GLFW_KEY_N: return Keyboard::N;		case GLFW_KEY_O: return Keyboard::O;
		case GLFW_KEY_P: return Keyboard::P;		case GLFW_KEY_Q: return Keyboard::Q;		case GLFW_KEY_R: return Keyboard::R;
		case GLFW_KEY_S: return Keyboard::S;		case GLFW_KEY_T: return Keyboard::T;		case GLFW_KEY_U: return Keyboard::U;
		case GLFW_KEY_V: return Keyboard::V;		case GLFW_KEY_W: return Keyboard::W;		case GLFW_KEY_X: return Keyboard::X;
		case GLFW_KEY_Y: return Keyboard::Y;		case GLFW_KEY_Z: return Keyboard::Z;
		case GLFW_KEY_ESCAPE: return Keyboard::Escape;		case GLFW_KEY_ENTER: return Keyboard::Enter;		case GLFW_KEY_TAB: return Keyboard::Tab;
		case GLFW_KEY_BACKSPACE: return Keyboard::Backspace;		case GLFW_KEY_INSERT: return Keyboard::Insert;		case GLFW_KEY_DELETE: return Keyboard::Delete;
		case GLFW_KEY_RIGHT: return Keyboard::Right;		case GLFW_KEY_LEFT: return Keyboard::Left;		case GLFW_KEY_DOWN: return Keyboard::Down;
		case GLFW_KEY_UP: return Keyboard::Up;		case GLFW_KEY_PAGE_UP: return Keyboard::PageUp;		case GLFW_KEY_PAGE_DOWN: return Keyboard::PageDown;
		case GLFW_KEY_HOME: return Keyboard::Home;		case GLFW_KEY_END: return Keyboard::End;
		case GLFW_KEY_F1: return Keyboard::F1;		case GLFW_KEY_F2: return Keyboard::F2;		case GLFW_KEY_F3: return Keyboard::F3;
		case GLFW_KEY_F4: return Keyboard::F4;		case GLFW_KEY_F5: return Keyboard::F5;		case GLFW_KEY_F6: return Keyboard::F6;
		case GLFW_KEY_F7: return Keyboard::F7;		case GLFW_KEY_F8: return Keyboard::F8;		case GLFW_KEY_F9: return Keyboard::F9;
		case GLFW_KEY_F10: return Keyboard::F10;		case GLFW_KEY_F11: return Keyboard::F11;		case GLFW_KEY_F12: return Keyboard::F12;
		case GLFW_KEY_KP_0: return Keyboard::NUM_KP0;		case GLFW_KEY_KP_1: return Keyboard::NUM_KP1;		case GLFW_KEY_KP_2: return Keyboard::NUM_KP2;
		case GLFW_KEY_KP_3: return Keyboard::NUM_KP3;		case GLFW_KEY_KP_4: return Keyboard::NUM_KP4;		case GLFW_KEY_KP_5: return Keyboard::NUM_KP5;
		case GLFW_KEY_KP_6: return Keyboard::NUM_KP6;		case GLFW_KEY_KP_7: return Keyboard::NUM_KP7;		case GLFW_KEY_KP_8: return Keyboard::NUM_KP8;
		case GLFW_KEY_KP_9: return Keyboard::NUM_KP9;
		}
		return Keyboard::Unknown;
	}
}


void Input::KeyboardCallback(void* p_window, int key, int, int action, int mod) noexcept
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(static_cast<GLFWwindow*>(p_window), GLFW_TRUE);
	}

	if (mod == GLFW_MOD_SHIFT)
		s_m_modifier = Modifier::Shift;
	else if (mod == GLFW_MOD_CONTROL)
		s_m_modifier = Modifier::Control;
	else if (mod == GLFW_MOD_ALT)
		s_m_modifier = Modifier::Alt;
	else
		s_m_modifier = Modifier::None;
	
	if (const int index = static_cast<int>(ToKeyboard(key)); action == GLFW_PRESS)
	{
		s_m_keyPress.set(index, true);
		s_m_keyRelease.set(index, false);
	}
	else if (action == GLFW_RELEASE)
	{
		s_m_keyPress.set(index, false);
		s_m_keyRelease.set(index, true);
	}
}

void Input::CursorPosCallback(void*, double x_pos, double y_pos) noexcept
{
	const glm::ivec2 pos = glm::ivec2(x_pos, s_m_windowSize.y - y_pos);
	if (s_m_isMouseDown[0] || s_m_isMouseDown[1])
		s_m_cursorDir = pos - s_m_cursor;
	s_m_cursor = pos;
	s_m_ray.x = 2.f * (static_cast<float>(pos.x) / static_cast<float>(s_m_windowSize.x)) - 1.f;
	s_m_ray.y = 2.f * (static_cast<float>(pos.y) / static_cast<float>(s_m_windowSize.y)) - 1.f;
	s_m_ray.z = -1;
}

void Input::MouseButtonCallback(void*, int button, int action, int mod) noexcept
{
	if (action == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
			s_m_isMouseDown[0] = false;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
			s_m_isMouseDown[1] = false;
		s_m_modifier = Modifier::None;
		s_m_cursorDir = glm::ivec2(0);
	}
	else
	{
		if (mod == GLFW_MOD_SHIFT)
			s_m_modifier = Modifier::Shift;
		else if (mod == GLFW_MOD_CONTROL)
			s_m_modifier = Modifier::Control;
		else if (mod == GLFW_MOD_ALT)
			s_m_modifier = Modifier::Alt;

		if (button == GLFW_MOUSE_BUTTON_LEFT)
			s_m_isMouseDown[0] = true;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
			s_m_isMouseDown[1] = true;
	}
}

void Input::ScrollCallback(void*, double, double) noexcept
{
}

void Input::DragAndDropCallback(void*, int count, const char** paths) noexcept
{
	for (int i = 0; i < count; ++i)
	{
		const std::filesystem::path file_path{ paths[i] };
		s_m_droppedPath.push_back(file_path);
		std::cout << "[Input]: Drag and Drop detected: " << file_path << std::endl;
	}
}

const glm::vec3& Input::GetNormalizedMousePos() noexcept
{
	return s_m_ray;
}

glm::ivec2 Input::GetMouseMovingDirection(MouseButton button) noexcept
{
	return s_m_isMouseDown[static_cast<int>(button)] ? s_m_cursorDir : glm::ivec2(0);
}

Modifier Input::GetModifier() noexcept
{
	return s_m_modifier;
}

bool Input::IsKeyPressed(Keyboard key) noexcept
{
	return s_m_keyPress[static_cast<int>(key)];
}

bool Input::IsKeyReleased(Keyboard key) noexcept
{
	if (s_m_keyRelease[static_cast<int>(key)])
	{
		s_m_keyRelease.set(static_cast<int>(key), false);
		return true;
	}
	return false;
}

bool Input::DropAndDropDetected() noexcept
{
	return !s_m_droppedPath.empty();
}

std::vector<std::filesystem::path> Input::GetDroppedPaths() noexcept
{
	std::vector<std::filesystem::path> paths = std::move(s_m_droppedPath);
	s_m_droppedPath.clear();
	return paths;
}
