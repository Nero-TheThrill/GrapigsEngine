/*
 *	Author		: Jina Hyun
 *	2nd Author  : Jinwoo Choi
 *	Date		: 09/30/22
 *	File Name	: Shader.cpp
 *	Desc		: Create shader and shader program
 */
#include "Shader.h"

#include <iostream>	// std::cout
#include <fstream>	// std::ifstream
#include <gl/glew.h>	// gl functions

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	// load png

namespace
{
	constexpr GLenum ToGLenum(const ShaderType type)
	{
		switch (type)
		{
		case ShaderType::None: return GL_NONE;
		case ShaderType::Vertex: return GL_VERTEX_SHADER;
		case ShaderType::Fragment: return GL_FRAGMENT_SHADER;
		case ShaderType::Geometry: return GL_GEOMETRY_SHADER;
		case ShaderType::Tessellation_Control: return GL_TESS_CONTROL_SHADER;
		case ShaderType::Tessellation_Evaluation: return GL_TESS_EVALUATION_SHADER;
		}
		return GL_NONE;
	}
	
}

/* Shader - start -------------------------------------------------------------------------------*/

Shader::Shader(ShaderType type, const std::filesystem::path& file_path) noexcept
	: m_type(type), m_filePath(file_path)
{
	Compile();
}

Shader::~Shader() noexcept
{
	Clear();
}

void Shader::Compile() noexcept
{
	if (m_isCompiled)
		return;

	std::ifstream ifs(m_filePath, std::ios::in);
	if (!ifs.is_open())
	{
		std::cout << "[Shader]: Cannot open " + m_filePath.string() << std::endl;
		return;
	}

	std::stringstream buffer;
	buffer << ifs.rdbuf();
	ifs.close();

	// create shader
	if (m_handle <= 0)
	{
		m_handle = glCreateShader(ToGLenum(m_type));
		if (m_handle == 0)
		{
			std::cout << "[Shader]: Unable to create a shader from " + m_filePath.string() << std::endl;
			return;
		}
	}

	// compile shader
	const std::string& contents = buffer.str();
	GLchar const* source[]{ contents.c_str() };
	glShaderSource(m_handle, 1, source, nullptr);
	glCompileShader(m_handle);

	// check compilation status
	GLint compiled = 0;
	glGetShaderiv(m_handle, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		GLint log_length = 0;
		glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &log_length);
		std::vector<char> error_log(log_length);
		glGetShaderInfoLog(m_handle, log_length, nullptr, error_log.data());
		Clear();
		std::cout << "[Shader]: <" << m_filePath << "> :Compile Failure" << std::endl;
		std::cout << std::string(error_log.begin(), error_log.end()) << std::endl;
		return;
	}
	m_isCompiled = compiled;
}

void Shader::Clear() noexcept
{
	if (m_handle > 0)
	{
		glDeleteShader(m_handle);
		m_handle = 0;
		m_isCompiled = false;
	}
}

unsigned Shader::GetHandle() const noexcept
{
	return m_handle;
}

bool Shader::IsCompiled() const noexcept
{
	return m_isCompiled;
}

/* Shader - end ---------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* ShaderProgram - start ------------------------------------------------------------------------*/

ShaderProgram::ShaderProgram(const std::vector<std::pair<ShaderType, std::filesystem::path>>& shader_files) noexcept
{
	for(const auto& shader : shader_files)
	{
		m_shader.push_back(new Shader(shader.first, shader.second));
	}
	LinkAndValidate();
}

ShaderProgram::~ShaderProgram() noexcept
{
	Clear();
	for (const auto& shader : m_shader)
		delete shader;
	m_shader.clear();
}

void ShaderProgram::Use() const noexcept
{
	glUseProgram(m_handle);
}

void ShaderProgram::UnUse() const noexcept
{
	glUseProgram(0);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, bool value) const noexcept
{
	glUniform1i(GetUniformLocation(uniform_name), value);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, int value) const noexcept
{
	glUniform1i(GetUniformLocation(uniform_name), value);
}
void ShaderProgram::SendUniform(const std::string& uniform_name, unsigned value) const noexcept
{
	glUniform1i(GetUniformLocation(uniform_name), value);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, float value) const noexcept
{
	glUniform1f(GetUniformLocation(uniform_name), value);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, const glm::vec2& value) const noexcept
{
	glUniform2f(GetUniformLocation(uniform_name), value.x, value.y);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, const glm::vec3& value) const noexcept
{
	glUniform3f(GetUniformLocation(uniform_name), value.x, value.y, value.z);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, const glm::vec4& value) const noexcept
{
	glUniform4fv(GetUniformLocation(uniform_name), 1, &value[0]);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, const glm::mat4& value) const noexcept
{
	glUniformMatrix4fv(GetUniformLocation(uniform_name), 1, GL_FALSE, &value[0][0]);
}


void ShaderProgram::Clear() noexcept
{
	if (m_handle > 0)
	{
		glDeleteProgram(m_handle);
		m_handle = 0;
		m_isLinked = false;
	}
	uniforms.clear();
}

int ShaderProgram::GetUniformLocation(const std::string& uniform_name) const noexcept
{
	if (const auto find = uniforms.find(uniform_name); find != uniforms.end())
		return find->second;

	int location = glGetUniformLocation(m_handle, uniform_name.c_str());
	if (location < 0)
	{
		std::cout << "[ShaderProgram]: <name=" << m_name << "> There isn't uniform variable <" << uniform_name << ">" << std::endl;
		location = -1;
	}
	uniforms[uniform_name] = location;
	return location;
}

void ShaderProgram::LinkAndValidate() noexcept
{
	if (m_isLinked)
		return;

	if (m_handle <= 0)
	{
		m_handle = glCreateProgram();
		if (m_handle == 0)
		{
			std::cout << "[ShaderProgram]: <name=" << m_name << "> Unable to create shader program" << std::endl;
			return;
		}
	}

	for (const auto& shader : m_shader)
	{
		if (shader->IsCompiled() == false)
		{
			std::cout << "[ShaderProgram]: <name=" << m_name << "> shader isn't compiled" << std::endl;
			return;
		}
 		glAttachShader(m_handle, shader->GetHandle());
	}

	// verify link status
	glLinkProgram(m_handle);
	GLint linked = 0;
	glGetProgramiv(m_handle, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLint log_length = 0;
		glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &log_length);
		std::vector<char> error_log(log_length);
		glGetProgramInfoLog(m_handle, log_length, nullptr, error_log.data());
		Clear();
		std::cout << "[ShaderProgram]: <name=" << m_name << "> Link Failure\n" + std::string(error_log.begin(), error_log.end()) << std::endl;
		return;
	}
	// Check validate
	glValidateProgram(m_handle);
	GLint is_validate = 0;
	glGetProgramiv(m_handle, GL_VALIDATE_STATUS, &is_validate);
	if (is_validate == GL_FALSE)
	{
		GLint log_length = 0;
		glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &log_length);
		std::vector<char> error_log(log_length);
		glGetProgramInfoLog(m_handle, log_length, nullptr, error_log.data());
		Clear();
		std::cout << "[ShaderProgram]: <name=" << m_name << "> Validate Failure\n" + std::string(error_log.begin(), error_log.end()) << std::endl;
		return;
	}
	m_isLinked = true;
}

void ShaderProgram::PrintActiveAttributes() const noexcept
{
	GLint max_length = 0, number = 0;
	glGetProgramiv(m_handle, GL_ACTIVE_ATTRIBUTES, &number);
	if (number <= 0)
		return;
	glGetProgramiv(m_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_length);
	std::vector<char> attrib;
	std::cout << "Location\t| Attribute Name\n------------------------------------" << std::endl;
	for (GLint i = 0; i < number; ++i)
	{
		attrib.resize(max_length);
		GLint size; GLenum type;
		glGetActiveAttrib(m_handle, i, max_length, nullptr, &size, &type, attrib.data());
		std::string str(attrib.begin(), attrib.end());
		std::cout << glGetAttribLocation(m_handle, str.c_str()) << "\t\t" << str << std::endl;
		attrib.clear();
	}
	std::cout << std::endl;
}

void ShaderProgram::PrintActiveUniforms() const noexcept
{
	GLint max_length = 0, number = 0;
	glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &number);
	if (number <= 0)
		return;
	glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
	std::vector<char> unifs;
	std::cout << "Location\t| Uniform Name\n------------------------------------" << std::endl;
	for (GLint i = 0; i < number; ++i)
	{
		unifs.resize(max_length);
		GLint size; GLenum type;
		glGetActiveUniform(m_handle, i, max_length, nullptr, &size, &type, unifs.data());
		const std::string str(unifs.begin(), unifs.end());
		std::cout << glGetUniformLocation(m_handle, str.c_str()) << "\t\t" << str << std::endl;
		unifs.clear();
	}
	std::cout << std::endl;
}

/* ShaderProgram - end --------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* Texture - start ------------------------------------------------------------------------------*/

unsigned Texture::s_textureCount = 1;

Texture::Texture(const char* file_path) noexcept
	: m_initialized(false), m_filePath(file_path)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, channels;
	unsigned char* data = stbi_load(m_filePath.string().c_str(), &width, &height, &channels, 0);
	if (data == nullptr)
	{
		std::cout << "[Texture] Error: Unable to load " << m_filePath << std::endl;
		return;
	}

	const GLenum sized_internal_format = (channels == 4) ? GL_RGBA8 : GL_RGB8;
	const GLenum base_internal_format = (channels == 4) ? GL_RGBA : GL_RGB;

	if (!m_handle)
		glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
	glTextureStorage2D(m_handle, 1, sized_internal_format, width, height);
	glTextureSubImage2D(m_handle, 0, 0, 0, width, height, base_internal_format, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	m_unit = s_textureCount++;
	glBindTextureUnit(m_unit, m_handle);
	const_cast<bool&>(m_initialized) = true;
}

Texture::~Texture() noexcept
{
	glDeleteTextures(1, &m_handle);
	m_handle = 0;
	m_unit = 0;
}

unsigned Texture::Unit() const noexcept
{
	return m_unit;
}


FrameBufferObject::FrameBufferObject() 
	: m_fboHandle(0), m_rboHandle(0), m_texture(0)
{
}
void FrameBufferObject::Init(int width, int height) noexcept
{
	if (!m_fboHandle)
		glCreateFramebuffers(1, &m_fboHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle);

	if (!m_texture)
		glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_INT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (!m_rboHandle)
		glGenRenderbuffers(1, &m_rboHandle);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rboHandle);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboHandle);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "[Frame Buffer Object]: Frame Buffer isn't complete" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferObject::Clear() noexcept
{
	glDeleteTextures(1, &m_texture);
	glDeleteFramebuffers(1, &m_fboHandle);
	glDeleteRenderbuffers(1, &m_rboHandle);
	m_texture = m_fboHandle = m_rboHandle = 0;
}

void FrameBufferObject::Bind() const noexcept
{
	static constexpr GLenum buffers[2] = { GL_COLOR_ATTACHMENT0,GL_DEPTH_ATTACHMENT };
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	glDrawBuffers(2, buffers);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBufferObject::UnBind() const noexcept
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned FrameBufferObject::GetTexture() const noexcept
{
	return m_texture;
}

/* Texture - end --------------------------------------------------------------------------------*/
