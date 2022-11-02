/*
 *	Author		: Jina Hyun
 *	2nd Author  : Jinwoo Choi
 *	Date		: 09/30/22
 *	File Name	: Shader.h
 *	Desc		: Create shader and shader program
 */
#pragma once

#include <filesystem>	// std::filesystem::path
#include <map>			// std::map
#include <glm/glm.hpp>	// glm


enum class ShaderType
{
	None, Vertex, Fragment, Geometry, Tessellation_Control, Tessellation_Evaluation
};

class Shader
{
public:
	Shader(ShaderType type, const std::filesystem::path& file_path) noexcept;
	~Shader() noexcept;
	void Compile() noexcept;
	void Clear() noexcept;
	[[nodiscard]] unsigned GetHandle() const noexcept;
	[[nodiscard]] bool IsCompiled() const noexcept;
private:
	bool m_isCompiled = false;
	ShaderType m_type = ShaderType::None;
	unsigned m_handle = 0;
	const std::filesystem::path& m_filePath;
};

class ShaderProgram
{
public:
	ShaderProgram(const std::vector<std::pair<ShaderType, std::filesystem::path>>& shader_files) noexcept;
	~ShaderProgram() noexcept;

	void Use() const noexcept;
	void UnUse() const noexcept;

	void SendUniform(const std::string& uniform_name, bool value) const noexcept;
	void SendUniform(const std::string& uniform_name, int value) const noexcept;
	void SendUniform(const std::string& uniform_name, unsigned value) const noexcept;
	void SendUniform(const std::string& uniform_name, float value) const noexcept;
	void SendUniform(const std::string& uniform_name, const glm::vec2& value) const noexcept;
	void SendUniform(const std::string& uniform_name, const glm::vec3& value) const noexcept;
	void SendUniform(const std::string& uniform_name, const glm::vec4& value) const noexcept;
	void SendUniform(const std::string& uniform_name, const glm::mat4& value) const noexcept;

	void PrintActiveAttributes() const noexcept;
	void PrintActiveUniforms() const noexcept;

	std::string m_name = "nan";
	const unsigned m_tag = 0;
private:
	void Clear() noexcept;
	[[nodiscard]] int GetUniformLocation(const std::string& uniform_name) const noexcept;
	void LinkAndValidate() noexcept;

	bool m_isLinked = false;
	unsigned m_handle = 0;
	std::vector<Shader*> m_shader;
	mutable std::map<std::string, int> uniforms;
};

class Texture
{
	static unsigned s_textureCount;
public:
	Texture(const char* file_path) noexcept;
	~Texture() noexcept;

	[[nodiscard]] unsigned Unit() const noexcept;

	const bool m_initialized;
	const unsigned m_tag = 0;
	const std::string m_name;
	const std::filesystem::path m_path;
private:
	unsigned m_handle = 0;
	unsigned m_unit = 0;
};

class FrameBufferObject
{
public:
	FrameBufferObject();
	~FrameBufferObject();

	void Init(int width, int height) noexcept;
	void Clear() noexcept;
	void Bind() const noexcept;
	void UnBind() const noexcept;

	[[nodiscard]] unsigned GetTexture() const noexcept;
private:
	const unsigned unit = 0;
	unsigned m_fboHandle, m_rboHandle, m_texture;
};