/*
 *	Author		: Jina Hyun
 *	Date		: 09/16/22
 *	File Name	: Input.h
 *	Desc		: Input system
 */

#pragma once
#include <glm/glm.hpp>	// glm

enum class MouseButton
{
	Left = 0, Right = 1
};

enum class ModKey
{
	None = 0, Shift, Control, Alt
};

class Input
{
public:
	static void KeyboardCallback(void* p_window, int key, int scancode, int action, int mods) noexcept;
	static void CursorPosCallback(void* p_window, double x_pos, double y_pos) noexcept;
	static void MouseButtonCallback(void* p_window, int button, int action, int mods) noexcept;
	static void ScrollCallback(void* p_window, double x_offset, double y_offset) noexcept;

	static glm::ivec2 GetMouseMovingDirection(MouseButton button) noexcept;
	static ModKey GetMouseMod() noexcept;

	static const glm::ivec2 s_m_windowSize;
private:
	static bool s_m_isMouseDown[2];
	static ModKey s_m_mouseMod;
	static glm::ivec2 s_m_cursorDir;
	static glm::ivec2 s_m_cursor;
};
