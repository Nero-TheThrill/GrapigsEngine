/*
 *	Author		: Jina Hyun
 *	Date		: 09/05/22
 *	File Name	: Application.cpp
 *	Desc		: Window system
 */
#include "Application.h"

#include <gl/glew.h>	// gl functions
#include <GLFW/glfw3.h>	// glfw functions
#include <iostream>		// std::cout
#include <stdexcept>	// std::runtime_error
#include <filesystem>	// std::filesystem::path

// ImGui functions
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Camera.h"	// CameraBuffer
#include "Input.h"	// Input

namespace Callback
{
	void Error(int error, const char* description) noexcept;
	void Keyboard(void* p_window, int key, int scancode, int action, int mod) noexcept;
	void WindowSizeChanged(void* p_window, int width, int height) noexcept;
	void DragAndDrop(void* p_window, int count, const char** paths) noexcept;
	void OpenGLDebug(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* user_param);
}

Application::Application(int width, int height, const char* title)
{
	if (width <= 0 || height <= 0)
		throw std::runtime_error("[Application] Error: Window size should be greater than 0");
	if (!glfwInit())
		throw std::runtime_error("[GLFW] Error: Unable to initialize GLFW");
	glfwSetErrorCallback(Callback::Error);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create windows
	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (window == nullptr)
		throw std::runtime_error("GLFW Error: Unable to create the window");
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable v-sync
	Input::s_m_windowSize = glm::ivec2(width, height);

	// Init GLEW
	if (glewInit() == GLEW_OK)
	{
		if (GLEW_VERSION_4_6)
		{
			std::cout << "Using GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;
			std::cout << "Driver supports OpenGL 4.6\n\n";
		}
		else
			throw std::runtime_error("[GLEW] Error: Driver does not support OpenGL 4.6");

		glViewport(0, 0, width, height);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
	}
	else
	{
		throw std::runtime_error("[GLEW] Error: Unable to initialize GLEW");
	}
	m_p_window = window;


	// Set callback
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h)
		{
			glViewport(0, 0, w, h);
			Input::s_m_windowSize = glm::ivec2(w, h);
			CameraBuffer::s_m_aspectRatio = static_cast<float>(w) / static_cast<float>(h);
		});
	glfwSetKeyCallback(window, [](GLFWwindow* p_win, int k, int s, int a, int m) {Input::KeyboardCallback(p_win, k, s, a, m); });
	glfwSetDropCallback(window, [](GLFWwindow* p_win, int c, const char** p){	Input::DragAndDropCallback(p_win, c, p);	});
	glfwSetCursorPosCallback(window, [](GLFWwindow* p_win, double x, double y) {Input::CursorPosCallback(p_win, x, y); });
	glfwSetMouseButtonCallback(window, [](GLFWwindow* p_win, int b, int a, int m) {Input::MouseButtonCallback(p_win, b, a, m); });
	glfwSetScrollCallback(window, [](GLFWwindow* p_win, double x, double y) {Input::ScrollCallback(p_win, x, y); });

	// ImGui Init
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
	ImGui_ImplOpenGL3_Init("#version 460");

	CameraBuffer::s_m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	CameraBuffer::SetMainCamera(new Camera());
	CameraBuffer::GetMainCamera()->Set(glm::vec3{ 0, 1, 3 });
}


bool Application::ShouldQuit() const noexcept
{
	return glfwWindowShouldClose(static_cast<GLFWwindow*>(m_p_window));
}

void Application::BeginUpdate() const noexcept
{
	// Window
	glfwPollEvents();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	CameraBuffer::UpdateMainCamera();
	CameraBuffer::Bind();
}

void Application::EndUpdate() const noexcept
{
	// ImGui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}

	// Window
	glfwSwapBuffers(static_cast<GLFWwindow*>(m_p_window));
}

void Application::CleanUp() const noexcept
{
	// Destroy
	CameraBuffer::Clear();

	// ImGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// GLFW
	glfwDestroyWindow(static_cast<GLFWwindow*>(m_p_window));
	glfwTerminate();
}

void Application::EnableOpenGLDebugCallback() noexcept
{
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glDebugMessageCallback(Callback::OpenGLDebug, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	glEnable(GL_DEBUG_OUTPUT);
}

void Application::SetBackgroundColor(byte red, byte green, byte blue)
{
	glClearColor(static_cast<float>(red) / 255.f, static_cast<float>(green) / 255.f, static_cast<float>(blue) / 255.f, 1.0);
}

namespace Callback
{
	void Error(int error, const char* description) noexcept
	{
		std::cerr << "[GLFW]: Error(" << error << ") : " << description << std::endl;
	}

	void Keyboard(void* p_window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mod) noexcept
	{
		if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
		{
			glfwSetWindowShouldClose(static_cast<GLFWwindow*>(p_window), GLFW_TRUE);
		}
	}

	void WindowSizeChanged([[maybe_unused]] void* p_window, int weight, int height) noexcept
	{
		if (weight == 0 || height == 0)
			return;
		glViewport(0, 0, weight, height);
		CameraBuffer::s_m_aspectRatio = static_cast<float>(weight) / static_cast<float>(height);
		Input::s_m_windowSize = glm::ivec2(weight, height);
	}

	void OpenGLDebug(unsigned source, unsigned type, [[maybe_unused]] unsigned id, unsigned severity, [[maybe_unused]] int length,
		const char* message, [[maybe_unused]] const void* user_param)
	{
		std::stringstream output;
		output << "OpenGL ";
		switch (source)
		{
		case GL_DEBUG_SOURCE_API:				output << "API";				break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		output << "Window System";		break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:	output << "Shader Compiler";	break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:		output << "Third Party";		break;
		case GL_DEBUG_SOURCE_APPLICATION:		output << "Application";		break;
		default:								break;
		}
		output << " " << (type == GL_DEBUG_TYPE_ERROR ? "Error(" : "(") << type << ")" << " [severity: ";
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:	output << "high";	break;
		case GL_DEBUG_SEVERITY_MEDIUM:	output << "medium";	break;
		case GL_DEBUG_SEVERITY_LOW:		output << "low";		break;
		default: /* Notification */		return;
		}
		output << "]: " << message << std::endl;
		std::cout << output.str();
	}
}
