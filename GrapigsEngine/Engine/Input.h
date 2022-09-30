/*
 *	Author		: Jina Hyun
 *	Date		: 09/16/22
 *	File Name	: Input.h
 *	Desc		: Input system
 */

#pragma once
#include <bitset>	// std::bitset
#include <glm/glm.hpp>	// glm

enum class MouseButton
{
	Left = 0, Right = 1
};

enum class Modifier
{
	None = 0, Shift, Control, Alt
};

enum class Keyboard
{
	Space = 0,
	Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	Escape, Enter, Tab, Backspace, Insert, Delete, Right, Left, Down, Up, PageUp, PageDown, Home, End,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, Unknown
};

class Input
{
public:
	static void KeyboardCallback(void* p_window, int key, int scancode, int action, int mods) noexcept;
	static void CursorPosCallback(void* p_window, double x_pos, double y_pos) noexcept;
	static void MouseButtonCallback(void* p_window, int button, int action, int mods) noexcept;
	static void ScrollCallback(void* p_window, double x_offset, double y_offset) noexcept;

	static glm::ivec2 GetMouseMovingDirection(MouseButton button) noexcept;
	static Modifier GetModifier() noexcept;
	static bool IsKeyPressed(Keyboard key) noexcept;

	static const glm::ivec2 s_m_windowSize;
private:
	static bool s_m_isMouseDown[2];
	static Modifier s_m_modifier;
	static glm::ivec2 s_m_cursorDir;
	static glm::ivec2 s_m_cursor;
	static std::bitset<static_cast<size_t>(Keyboard::Unknown) + 1> s_m_keyPress;
};
