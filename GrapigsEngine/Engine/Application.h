/*
 *	Author		: Jina Hyun
 *	Date		: 09/05/22
 *	File Name	: Application.cpp
 *	Desc		: Window system
 */
#pragma once

using byte = unsigned char;
class Application
{
public:
	Application(int width = 1000, int height = 800, const char* title = "Grapigs Engine");
	[[nodiscard]] bool ShouldQuit() const noexcept;

	void BeginUpdate() const noexcept;
	void EndUpdate() const noexcept;

	void CleanUp() const noexcept;

	static void EnableOpenGLDebugCallback() noexcept;
	static void SetBackgroundColor(byte red, byte green, byte blue);
private:
	void* m_p_window = nullptr;
};

